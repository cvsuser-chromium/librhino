#include <ithread.h>
#include <fcntl.h>

#include "upnp.h"
#include "upnp_render.h"
#include "upnp_util_dmr.h"

#include "hitTime.h"
#include "dlna_api.h"
#include "dlna_api_cpp.h"
#include "dlna_api_raw.h"
#include "json_tokener.h"
//#include "Network.h"
//#include "Application.h"
#include <base/strings/stringprintf.h>
#include <string>

#define  value_free(value) \
						if(value){	 \
							free(value);\
							value = NULL;\
						}


#define render_track() \
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_FEW, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__)

#define render_track_with_level(x) \
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_FEW, "\n")
#if defined(OS_WIN)
static void HT_Printf(enum_HT_MODULES module, enum_HT_BITS Level,
          const char *fileName, int lineNo, const char *funcName,
  				const char* fmt, ...)
{

}

#endif

#if defined(ENABLE_HT_PRINTF) || defined(DEBUG)
#define render_msg(fmt, ... ) \
	HT_Printf(HT_MOD_DMR, HT_BIT_FEW, __FILE__, __LINE__, __FUNCTION__, fmt)
#else
#define render_msg(fmt, ... ) {}
#endif

#define min(a, b)   (((a)<(b))? (a):(b))

//t_DMR_AVP g_dmr_player = {0};
static ithread_mutex_t render_mux;
static UpnpDevice_Handle render_handle = -1;
static int advr_expire = 1880;

/*   Device type for render device  */
//static char render_device_type[] = "urn:schemas-upnp-org:device:MediaRenderer:1";

/*   Service types for render services */
static const char *render_service_type[] = { 	"urn:schemas-upnp-org:service:AVTransport:1",
								"urn:schemas-upnp-org:service:ConnectionManager:1",
								"urn:schemas-upnp-org:service:RenderingControl:1",
								"urn:schemas-upnp-org:service:X-CTC_Subscribe:1"
								};
static char DMR_name[128] = {0};
static char DMR_path[128] = {0};
static char DMR_mac_addr[128] = {0};

#define AVTRANSPORT_VAR_CNT 128
//static struct upnp_var AVTransport_var[AVTRANSPORT_VAR_CNT];
#define AVTRANSPORT_ACTION_CNT 128
static struct upnp_action AVTransport_action[AVTRANSPORT_ACTION_CNT];

#define CONNECTIONMANAGER_VAR_CNT 128
//static struct upnp_var ConnectionManager_var[CONNECTIONMANAGER_VAR_CNT];
#define CONNECTIONMANAGER_ACTION_CNT 128
static struct upnp_action ConnectionManager_action[CONNECTIONMANAGER_ACTION_CNT];

#define RENDERINGCONTROL_VAR_CNT 128
//static struct upnp_var RenderingControl_var[RENDERINGCONTROL_VAR_CNT];
#define RENDERINGCONTROL_ACTION_CNT 128
static struct upnp_action RenderingControl_action[RENDERINGCONTROL_ACTION_CNT];

#define X_CTC_SUBSCRIBE_VAR_CNT 128
//static struct upnp_var X_CTC_SUBSCRIBE_var[RENDERINGCONTROL_VAR_CNT];
#define X_CTC_SUBSCRIBE_ACTION_CNT 128
static struct upnp_action X_CTC_SUBSCRIBE_action[RENDERINGCONTROL_ACTION_CNT];


#define RENDER_SERVICE_CNT 4
static struct upnp_service upnp_service_table[RENDER_SERVICE_CNT];

#define AVTRANSPORT_SERVICE 0
#define CONNECTIONMANAGER_SERVICE 1
#define RENDERINGCONTROL_SERVICE 2
#define X_CTC_SUBSCRIBE_SERVICE 3

namespace {
DMREVRNT_CALLBACK_CPP 	DMREvent_Callback = NULL;
}
#define UPNP_CLASS_LEN 32
#define DC_TITLE_LEN 32
#define DC_DATE_LEN 12
#define RESTRICTED_LEN 1
#define MEDIA_CLASS_LEN 32
#define OBJECT_ID_LEN 36
#define PARENT_ID_LEN 10

enum M_TYPE_T{
	M_UNKNOWN = 0,
	M_PICTURE = 1,
	M_AUDIO = 2,
	M_VIDEO = 3
};
struct image_info{
	int colorDepth;
	char *resolution;
};
struct media_detail{
	char *contentUri;
	char *importUri;
	char *protocolInfo;
	char *encryptKey;
	int keyByteLen;
	int size;
	union {
		int duration;
		struct image_info image;
	}u;
};
struct media_property{
	enum M_TYPE_T media_type;
	char upnp_class[UPNP_CLASS_LEN + 1];
	char dc_title[DC_TITLE_LEN + 1];
	char dc_date[DC_DATE_LEN + 1];
	char restricted[RESTRICTED_LEN + 1];
	char media_class[MEDIA_CLASS_LEN + 1];
	char object_id[OBJECT_ID_LEN + 1];
	char parent_id[PARENT_ID_LEN + 1];
	struct media_detail m_detail;
};

 int media_play(char *speed);
 int media_stop(void);
 int media_pause(void);
 int media_fast_forward(int level);
 int media_fast_rewind(int level);
 int media_seek(long long  target);


extern int SetAVTransportURI( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int Player_Previous( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetCurrentTransportActions( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int SetPlayMode( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetMediaInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetPositionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetTransportInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int GetTransportSettings( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
extern int Player_Next( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );


int GetDeviceCapabilities( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int Player_Pause( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int Player_Play( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int Player_Seek( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );

int X_DLNA_GetBytePositionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int ConnectionComplete( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int PrepareForConnection( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );

int Player_Stop( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetCurrentConnectionIDs( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetCurrentConnectionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetProtocolInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetBlueVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetBrightness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetColorTemperature( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetContrast( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetGreenVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetMute( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetRedVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetSharpness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetVolume( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int ListPresets( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SelectPreset( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetBlueVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetBrightness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetColorTemperature( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetContrast( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetGreenVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetMute( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetRedVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetSharpness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetVolume( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int GetAllowedTransforms( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int X_CTC_GetProductInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int X_CTC_Order( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int SetTransforms( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString );
int RenderHandleSubscriptionRequest( IN struct Upnp_Subscription_Request *sr_event );
int RenderHandleGetVarRequest( INOUT struct Upnp_State_Var_Request *cgv_event );
int RenderHandleActionRequest( INOUT struct Upnp_Action_Request *ca_event );
int av_render_name_get(char *render_name,  int len);
int av_render_name_set(char *render_name);
int render_start(char *ip, int port);
void render_stop(void);
void render_dbg_msg_enable(int enable);

static void render_player_state(int state)
{
	state = 0;
}

static struct media_property property;

static char *element_get(char *str, const char *element_name)
{
	char *ele_begin, *ele_end, *tmp_ptr;
	int begin_found = 0;//, end_found = 0;
	char *begin_str = (char *)malloc(strlen(element_name) + strlen("<") + 1);
	int tail_len = strlen(element_name) + strlen("</>") ;
	char *end_str = (char *)malloc(tail_len + 1);
	char *ret;

	sprintf(begin_str, "<%s", element_name);
	sprintf(end_str, "</%s>", element_name);
	ele_begin = strstr(str, begin_str);
	if( ele_begin ){
			begin_found = 1;
	}
	value_free(begin_str);
	if(!begin_found || (ele_begin + 1) >= (str + strlen(str)) ){
		return NULL;
	}
	tmp_ptr = strstr(ele_begin, "/>");
	ele_end = strstr(&ele_begin[1], end_str);
	value_free(end_str);
	if(ele_end){
		if(tmp_ptr && tmp_ptr < ele_end){
			ele_end = tmp_ptr;
			tail_len = 2;
		}
		ret = (char *)malloc(ele_end - ele_begin + tail_len + 2);
		strncpy(ret, ele_begin, ele_end - ele_begin + tail_len + 1);
		ret[ele_end - ele_begin + tail_len + 1] = 0;
		//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "element_get is %s\n", ret);
		return ret;
	}
	return NULL;
}

/*================================================================
*   copy_with_escape
*
*
*=================================================================*/
static int xml_change_copy_cpp(std::string& dst_buf, int dst_buf_len, const char *p )
{
  int i = 0;
  int plen = 0;
  int dst_shift = 0;
  int step = 0;

  if( p == NULL )
    return -1;
  plen = strlen( p );

  for( i = 0; i < plen; i++ ) {
    switch ( p[i] ) {
    case '<':
      dst_buf += "&lt;";
      break;
    case '>':
       dst_buf += "&gt;";
      break;
    case '&':
      dst_buf += "&amp;";
      break;
    case '\'':
      dst_buf += "&apos;";
      break;
    case '\"':
      dst_buf += "&quot;";
      break;
    default:
      dst_buf += p[i];
      break;
    }
  }
  return 0;
}
static char *element_value_get(char *str, const char *element_name)
{
	char *end_match = (char *)malloc(strlen(element_name) + 3 + 1);
	char *value_begin, *value_end;
	char *ret_value;

	sprintf(end_match, "</%s>", element_name);
	value_end = strstr(str, end_match);
	value_free(end_match);
	if(!value_end || (value_end - 1) < str)
		return NULL;
	value_end --;
	value_begin = strchr(str, '>');
	if(!value_begin || value_begin >= value_end){
		return NULL;
	}
	value_begin ++;
	while((value_begin) < (str + strlen(str))){
		if(value_begin[1] == '\r' || value_begin[1] == '\n' || value_begin[1] == ' ')
			value_begin ++;
		else
			break;
	}
	ret_value = (char *)malloc(value_end - value_begin + 2);
	strncpy(ret_value, value_begin, value_end - value_begin + 1);
	ret_value[value_end - value_begin + 1] = 0;
	//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "element_value_get is %s\n", ret_value);
	return ret_value;
}

static char *element_attr_get(char *str, const char *attr_name)
{
	char *attr_begin, *attr_end, *tmp_ptr;
	char *ret_value;

	attr_begin = strstr(str, attr_name);
	if(attr_begin){
		attr_begin += strlen(attr_name);
		while((++attr_begin) < (str + strlen(str))){
			if(*attr_begin == '=' || *attr_begin == ' ' || *attr_begin == '\"')
				continue;
			else
				break;
		}
		tmp_ptr = strchr(attr_begin, '>');
		attr_end = strstr(attr_begin, "\" ");
		if(attr_end){
			if(tmp_ptr && tmp_ptr < attr_end)
				attr_end = --tmp_ptr;
		} else {
			if(tmp_ptr)
				attr_end = tmp_ptr;
		}
		attr_end --;
		if(attr_end){
			ret_value = (char *)malloc(attr_end - attr_begin + 2);
			strncpy(ret_value, attr_begin, attr_end - attr_begin + 1);
			ret_value[attr_end - attr_begin + 1] = 0;
			//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "attr is %s\n", ret_value);
			return ret_value;
		}
	}
	return NULL;
}

//static pthread_mutex_t var_rw_mux;
static pthread_mutex_t var_mux;

static inline void upnp_var_init(struct upnp_service *p_service, int index, const char *var_name, char * var_value, char var_type)
{
	char data[11];

	ithread_mutex_lock(&var_mux);
	p_service->var_name[index] = (char *)var_name;
	if(var_value)
		p_service->var_value[index] = strdup(var_value);
	else {
		if(var_type == 0){
			p_service->var_value[index] = strdup("uninit");
		} else if(var_type == 1 || var_type == 2){
			sprintf(data, "%d", 0xffffffff);
			p_service->var_value[index] = strdup(data);
		} else if(var_type == 3 || var_type == 4){
			sprintf(data, "%d", 0xffff);
			p_service->var_value[index] = strdup(data);
		} else if(var_type == 5){
			p_service->var_value[index] = strdup("0");
		}
	}
	p_service->var_type[index] = var_type;
	ithread_mutex_unlock(&var_mux);
	return;
}

static inline int var_set_internal(struct upnp_service *p_service, const char *var_name, const char* var_value)
{
	int i;

	//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "var cnt is %d, name is %s\n", p_service->var_cnt, var_name);
	for(i = 0; i < p_service->var_cnt; i++){
		//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%s:%s:%d:%d\n", p_service->var_name[i], var_name, strlen(p_service->var_name[i]), strlen(var_name));
		if (p_service->var_name[i] == NULL) {
			break;
		}
		if (strcmp(p_service->var_name[i], var_name) == 0){
			if (p_service->var_value[i])
				value_free(p_service->var_value[i]);
			if (var_value)
				p_service->var_value[i] = strdup(var_value);
			else
				p_service->var_value[i] = strdup("uninit");
			return i;
		}
	}
	render_msg("var:%s is not found\n", var_name);
	return -1;
}

static inline int upnp_var_set(struct upnp_service *p_service, const char *var_name, const char* var_value)
{
//	int i;
	int ret;

	ithread_mutex_lock(&var_mux);
	ret = var_set_internal(p_service, var_name, var_value);
	ithread_mutex_unlock(&var_mux);
	return ret;
}

static inline int var_get_internal(struct upnp_service *p_service,const char *var_name, char **var_value)
{
	int i;

	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "var_name =%s \n",var_name);
	if(var_value)
		*var_value = NULL;

	for(i = 0; i < p_service->var_cnt; i++){
		if (p_service->var_name[i] == NULL) {
			break ;
		}
		if (strcmp(p_service->var_name[i], var_name) == 0){
			if (var_value){
				if (strcmp(p_service->var_value[i], "uninit") != 0)
  				*var_value = strdup(p_service->var_value[i]);
			}
			return i;
 		}
	}
	render_msg("var:%s is not found\n", var_name);
	return -1;
}


static inline int upnp_var_get(struct upnp_service *p_service, const char *var_name,  char **var_value)
{
//	int i;
	int ret;

	ithread_mutex_lock(&var_mux);
	ret = var_get_internal(p_service, var_name, var_value);
	ithread_mutex_unlock(&var_mux);
	return ret;
}

static int media_propertity_get(char *content, struct media_property *m_property)
{
  char *element, *value, *sub_value;
  int cp_len;

  render_track();
  element = element_get(content, "DIDL-Lite");
  if (!element)
  {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "element is NULL\n");
    return 0 ;
  }
  render_track();
  value = element_value_get(element, "DIDL-Lite");
  if(!value)
  {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY,"value NULL\n");
    return 0 ;
  }
  value_free(element);
  element = element_get(value, "item");
  if (!element)
  {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "element is NULL\n");
    return 0 ;
  }
  render_track();
  if(!value)
  {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY,"value NULL\n");
    return 0 ;
  }
  value_free(value);
  render_track();
  value = element_attr_get(element, "id");
  if(!value)
  {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY,"value NULL\n");
    return 0 ;
  }
  render_track();
  if(value){
    cp_len = min(strlen(value), OBJECT_ID_LEN);
    memcpy(m_property->object_id, value, cp_len);
    m_property->object_id[cp_len] = 0;
    //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "value is %s\n", value);
    value_free(value);
  }
  render_track();
  value = element_attr_get(element, "parentID");
  if(value){
    cp_len = min(strlen(value), PARENT_ID_LEN);
    memcpy(m_property->parent_id, value, cp_len);
    m_property->parent_id[cp_len] = 0;
    //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "value is %s\n", value);
    value_free(value);
  }
  render_track();
  value = element_attr_get(element, "restricted");
  if(value){
    cp_len = min(strlen(value), RESTRICTED_LEN);
    memcpy(m_property->restricted, value, cp_len);
    m_property->restricted[cp_len] = 0;
    //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "value is %s\n", value);
    value_free(value);
  }
  render_track();
  value = element_get(element, "dc:title");
  if(value){
    sub_value = element_value_get(value, "dc:title");
    if(sub_value){
      cp_len = min(strlen(sub_value), DC_TITLE_LEN);
      memcpy(m_property->dc_title, sub_value, cp_len);
      m_property->dc_title[cp_len] = 0;
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "dc:title is :%s\n", sub_value);
      value_free(sub_value);
    }
    value_free(value);
  }
  render_track();
  value = element_get(element, "upnp:class");
  if(value){
    sub_value = element_value_get(value, "upnp:class");
    if(sub_value){
      cp_len = min(strlen(sub_value), UPNP_CLASS_LEN);
      memcpy(m_property->upnp_class, sub_value, cp_len);
      m_property->upnp_class[cp_len] = 0;
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "upnp:classis :%s\n", sub_value);
      value_free(sub_value);
    }
    value_free(value);
  }
  render_track();
  value = element_get(element, "dc:date");
  if(value){
    sub_value = element_value_get(value, "dc:date");
    if(sub_value){
      cp_len = min(strlen(sub_value), DC_DATE_LEN);
      memcpy(m_property->dc_date, sub_value, cp_len);
      m_property->dc_date[cp_len] = 0;
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "dc:date is :%s\n", sub_value);
      value_free(sub_value);
    }
    value_free(value);
  }
  render_track();
  value = element_get(element, "res");
  if(value){
    sub_value = element_value_get(value, "res");
    if(sub_value){
      m_property->m_detail.contentUri = strdup(sub_value);
      value_free(sub_value);
    }
    render_track();
    sub_value = element_attr_get(value, "protocolInfo");
    if(sub_value){
      m_property->m_detail.protocolInfo = strdup(sub_value);
      upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ProtocolInfo", sub_value);
      value_free(sub_value);
    }
    render_track();
    sub_value = element_attr_get(value, "localEncrypted");
    m_property->m_detail.keyByteLen = 0;
    if(sub_value){
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "******** Have Defined Encryption\n");
      int localEncrypted = atoi(sub_value);
      value_free(sub_value);
      m_property->m_detail.keyByteLen = 0;
      if(localEncrypted)
      {
        HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "******** Have Defined Encryption and Encryption is valid\n");
        sub_value = element_attr_get(value, "localEncryptionKey");
        if(sub_value){
          m_property->m_detail.keyByteLen = strlen(sub_value);
          m_property->m_detail.encryptKey = strdup(sub_value);
          value_free(sub_value);
        }
      }
    }
    render_track();
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "******** m_property->m_detail.keyByteLen is %d, key = %s\n", m_property->m_detail.keyByteLen, m_property->m_detail.encryptKey);
    sub_value = element_attr_get(value, "size");
    if(sub_value){
      m_property->m_detail.size = atoi(sub_value);
      value_free(sub_value);
    }
    sub_value = element_attr_get(value, "importUri");
    if(sub_value){
      m_property->m_detail.importUri = strdup(sub_value);
      value_free(sub_value);
    }
    if(m_property->upnp_class[0] != 0){
      if (!strncmp(m_property->upnp_class, "object.item.imageItem", strlen("object.item.imageItem"))){
        sub_value = element_attr_get(value, "resolution");
        if(sub_value){
          m_property->m_detail.u.image.resolution = strdup(sub_value);
          value_free(sub_value);
        }
        sub_value = element_attr_get(value, "colorDepth");
        if(sub_value){
          m_property->m_detail.u.image.colorDepth = atoi(sub_value);
          value_free(sub_value);
        }
        m_property->media_type = M_PICTURE;
        HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "this is a image\n");
      } else if(!strncmp(m_property->upnp_class, "object.item.videoItem", strlen("object.item.videoItem"))){
        sub_value = element_attr_get(value, "duration");
        if(sub_value){
          //					char *ret;
          upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentMediaDuration", sub_value);
          upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackDuration", sub_value);
          //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackDuration", &ret);
          m_property->m_detail.u.duration = atoi(sub_value);
          value_free(sub_value);
        }
        m_property->media_type = M_VIDEO;
      } else if(!strncmp(m_property->upnp_class, "object.item.audioItem", strlen("object.item.audioItem"))){
        sub_value = element_attr_get(value, "duration");
        if(sub_value){
          upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentMediaDuration", sub_value);
          upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackDuration", sub_value);
          m_property->m_detail.u.duration = atoi(sub_value);
          value_free(sub_value);
        }
        m_property->media_type = M_AUDIO;
        HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "this is a audio\n");
      }
    }
    render_track();
    value_free(value);
  }
  render_track();
  value_free(element);
  return 0;
}

static int media_propertity_clear(struct media_property *m_property)
{
  if(m_property->media_type == M_PICTURE && m_property->m_detail.u.image.resolution){
    value_free(m_property->m_detail.u.image.resolution);
    m_property->m_detail.u.image.resolution = NULL;
  }
  if(m_property->m_detail.importUri){
    value_free(m_property->m_detail.importUri);
    m_property->m_detail.importUri = NULL;
  }
  if(m_property->m_detail.protocolInfo){
    value_free(m_property->m_detail.protocolInfo);
    m_property->m_detail.protocolInfo = NULL;
  }
  if(m_property->m_detail.contentUri){
    value_free(m_property->m_detail.contentUri);
    m_property->m_detail.contentUri = NULL;
  }
  if(m_property->m_detail.encryptKey){
    value_free(m_property->m_detail.encryptKey);
    m_property->m_detail.encryptKey = NULL;
  }
  m_property->media_type = M_UNKNOWN;
  memset(m_property, 0, sizeof(struct media_property));
  return 0;
}

static int transport_action_set(struct media_property *m_property)
{
  switch(m_property->media_type){
  case M_PICTURE:
    upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTransportActions", "Play,Stop");
    break;
  case M_AUDIO:
    if(m_property->m_detail.protocolInfo && strstr( m_property->m_detail.protocolInfo, "http-get:*:audio/L16")){
      upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTransportActions", "Play,Stop,Seek,Pause,X_DLNA_SeekByte");
      break;
    }
  case M_VIDEO:
    upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTransportActions", "Play,Stop,Pause,Seek,X_DLNA_SeekByte");
    break;
  case M_UNKNOWN:
    upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTransportActions", NULL);
    break;
  }
  return 0;
}

static void upnp_action_set(struct upnp_action *action, const char *action_name, upnp_xml_action callback)
{
	action->action_name = (char *)action_name;
	action->callback = callback;
}
static inline int render_property_add_cpp(int service_id, std::string& wr_ptr, const char *property_name)
{
  char *value = NULL,*value1 = NULL;
  int orig_len = wr_ptr.length();
  //char xml_change_buf[64 * 1024];
  std::string xml_change_buf;

  upnp_var_get(&upnp_service_table[service_id], property_name, &value);
  if (value) {
    //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "name is %s, value is %s\n", property_name, value);
    if(xml_change_copy_cpp(xml_change_buf, 64 * 1024, value) != 0){
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "error to change %s's value to xml format:\n%s\n", property_name, value);
      value_free(value);
      return -1;
    }
    if (0 == strcmp("Mute channel",property_name)) {
      upnp_var_get(&upnp_service_table[service_id], "Mute", &value1);
      //sprintf(wr_ptr, "<%s=\"%s\" val=\"%s\"/>\r\n", property_name, xml_change_buf,value1);
      base::StringAppendF(&wr_ptr, "<%s=\"%s\" val=\"%s\"/>\r\n", property_name, xml_change_buf.c_str(), value1);
      value_free(value1);
    } else if(0 == strcmp("Volume channel",property_name)) {
      upnp_var_get(&upnp_service_table[service_id], "Volume", &value1);
      base::StringAppendF(&wr_ptr, "<%s=\"%s\" val=\"%s\"/>\r\n", property_name, xml_change_buf.c_str(),value1);
      value_free(value1);
    } else {
      base::StringAppendF(&wr_ptr, "<%s val=\"%s\"/>\r\n", property_name, xml_change_buf.c_str());
    }
    value_free(value);
  } else {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "error to read %s\n", property_name);
    base::StringAppendF(&wr_ptr, "<%s/>\r\n", property_name);
  }
  return wr_ptr.length() - orig_len;
}

#define rcs_property_add_cpp(x, y) render_property_add_cpp(RENDERINGCONTROL_SERVICE, x, y)
#define property_add_cpp(x, y) render_property_add_cpp(AVTRANSPORT_SERVICE, x, y)
#define cms_property_add_cpp(x, y) render_property_add_cpp(CONNECTIONMANAGER_SERVICE, x, y)
#define x_ctc_ss_property_add_cpp(x, y) render_property_add_cpp(X_CTC_SUBSCRIBE_SERVICE, x, y)


static int rcs_lastchange_build_cpp(std::string& wr_ptr, size_t buf_len, char* id)
{
  int add_len = 0;
  int orig_len = wr_ptr.length();
  base::StringAppendF(&wr_ptr, "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/RCS/\">\r\n <InstanceID val =\"%s\">\r\n", id);
  add_len = rcs_property_add_cpp(wr_ptr, "PresetNameList");
  add_len = rcs_property_add_cpp(wr_ptr, "Brightness");
  add_len = rcs_property_add_cpp(wr_ptr, "Contrast");
  add_len = rcs_property_add_cpp(wr_ptr, "Sharpness");
  add_len = rcs_property_add_cpp(wr_ptr, "RedVideoGain");
  add_len = rcs_property_add_cpp(wr_ptr, "GreenVideoGain");
  add_len = rcs_property_add_cpp(wr_ptr, "BlueVideoGain");
  add_len = rcs_property_add_cpp(wr_ptr, "RedVideoBlackLevel");
  add_len = rcs_property_add_cpp(wr_ptr, "GreenVideoBlackLevel");
  add_len = rcs_property_add_cpp(wr_ptr, "BlueVideoBlackLevel");
  add_len = rcs_property_add_cpp(wr_ptr, "ColorTemperature");
  add_len = rcs_property_add_cpp(wr_ptr, "HorizontalKeystone");
  add_len = rcs_property_add_cpp(wr_ptr, "VerticalKeystone");
  add_len = rcs_property_add_cpp(wr_ptr, "VolumeDB");
  add_len = rcs_property_add_cpp(wr_ptr, "Mute channel");
  add_len = rcs_property_add_cpp(wr_ptr, "Volume channel");
  add_len = rcs_property_add_cpp(wr_ptr, "Loudness");
  wr_ptr += "</InstanceID>\r\n</Event>\r\n";
	return 0;
}

static int av_lastchange_build_cpp(std::string& wr_ptr, size_t buf_len, char* id)
{
  int orig_len = wr_ptr.length();

  base::StringAppendF(&wr_ptr, "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/AVT/\">\r\n <InstanceID val =\"%s\">\r\n", id);
  property_add_cpp(wr_ptr, "TransportState");
  property_add_cpp(wr_ptr, "TransportStatus");
  property_add_cpp(wr_ptr, "PlaybackStorageMedium");
  property_add_cpp(wr_ptr, "RecordStorageMedium");
  property_add_cpp(wr_ptr, "PossiblePlaybackStorageMedia");
  property_add_cpp(wr_ptr, "PossibleRecordStorageMedia");
  property_add_cpp(wr_ptr, "CurrentPlayMode");
  property_add_cpp(wr_ptr, "TransportPlaySpeed");
  property_add_cpp(wr_ptr, "RecordMediumWriteStatus");
  property_add_cpp(wr_ptr, "CurrentRecordQualityMode");
  property_add_cpp(wr_ptr, "PossibleRecordQualityModes");
  property_add_cpp(wr_ptr, "NumberOfTracks");
  property_add_cpp(wr_ptr, "CurrentTrack");
  property_add_cpp(wr_ptr, "CurrentTrackDuration");
  property_add_cpp(wr_ptr, "CurrentMediaDuration");
  property_add_cpp(wr_ptr, "CurrentTrackMetaData");
  property_add_cpp(wr_ptr, "CurrentTrackURI");
  property_add_cpp(wr_ptr, "AVTransportURI");
  property_add_cpp(wr_ptr, "AVTransportURIMetaData");
  property_add_cpp(wr_ptr, "CurrentTransportActions");
  wr_ptr += "</InstanceID>\r\n</Event>\r\n";

  return wr_ptr.length() - orig_len;
}

static int cms_lastchange_build_cpp(std::string& wr_ptr, size_t buf_len, char * id)
{
  base::StringAppendF(&wr_ptr, "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/RCS/\">\r\n <InstanceID val =\"%s\">\r\n", id);

  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_AVTransportID");
  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_ConnectionID");
  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_ConnectionManager");
  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_ConnectionStatus");
  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_Direction");
  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_ProtocolInfo");
  cms_property_add_cpp(wr_ptr, "A_ARG_TYPE_RcsID");
  cms_property_add_cpp(wr_ptr, "CurrentConnectionIDs");
  cms_property_add_cpp(wr_ptr, "SinkProtocolInfo");
  cms_property_add_cpp(wr_ptr, "SourceProtocolInfo");
  wr_ptr += "</InstanceID>\r\n</Event>\r\n";

  return 0;
}

static int x_ctc_ss_lastchange_build_cpp(std::string& wr_ptr, size_t buf_len, char * id)
{
  base::StringAppendF(&wr_ptr, "<Event xmlns = \"urn:schemas-upnp-org:metadata-1-0/RCS/\">\r\n <InstanceID val =\"%s\">\r\n", id);
  x_ctc_ss_property_add_cpp(wr_ptr, "ContentType");
  x_ctc_ss_property_add_cpp(wr_ptr, "ProductId");
  x_ctc_ss_property_add_cpp(wr_ptr, "ContentId");
  x_ctc_ss_property_add_cpp(wr_ptr, "ServiceId");
  x_ctc_ss_property_add_cpp(wr_ptr, "ColumnId");
  wr_ptr += "</InstanceID>\r\n</Event>\r\n";
  return 0;
}



/******************************************************************************
*       <name>GetCurrentTransportActions</name>
*         <argumentList>
*            <argument>
*               <name>InstanceID</name> <direction>in</direction>
*               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
*            </argument>
*            <argument>
*               <name>Actions</name> <direction>out</direction>
*               <relatedStateVariable>CurrentTransportActions</relatedStateVariable>
*            </argument>
*         </argumentList>
*******************************************************************************/
int GetCurrentTransportActions( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTransportActions", &value);
  if (UpnpAddToActionResponse( out, "GetCurrentTransportActions",
     render_service_type[AVTRANSPORT_SERVICE], "Actions", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  return UPNP_E_SUCCESS;
}

/*******************************************************************************
*         <name>GetDeviceCapabilities</name>
*         <argumentList>
*            <argument>
*               <name>InstanceID</name> <direction>in</direction>
*               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
*            </argument>
*            <argument>
*               <name>PlayMedia</name> <direction>out</direction>
*               <relatedStateVariable>PossiblePlaybackStorageMedia</relatedStateVariable>
*            </argument>
*            <argument>
*               <name>RecMedia</name>  <direction>out</direction>
*               <relatedStateVariable>PossibleRecordStorageMedia</relatedStateVariable>
*            </argument>
*            <argument>
*               <name>RecQualityModes</name>  <direction>out</direction>
*               <relatedStateVariable>PossibleRecordQualityModes</relatedStateVariable>
*            </argument>
*         </argumentList>
*********************************************************************************/
int GetDeviceCapabilities( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  //	int index = -1;

  render_track();
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "PossiblePlaybackStorageMedia", &value);
  if( UpnpAddToActionResponse( out, "GetDeviceCapabilities",
     render_service_type[AVTRANSPORT_SERVICE], "PlayMedia", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "PossibleRecordStorageMedia", &value);
  if( UpnpAddToActionResponse( out, "GetDeviceCapabilities",
    render_service_type[AVTRANSPORT_SERVICE], "RecMedia", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "PossibleRecordQualityModes", &value);
  if( UpnpAddToActionResponse( out, "GetDeviceCapabilities",
    render_service_type[AVTRANSPORT_SERVICE], "RecQualityModes", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free( value);
  return UPNP_E_SUCCESS;
}

/**********************************************************************
         <name>GetMediaInfo</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>NrTracks</name> <direction>out</direction>
               <relatedStateVariable>NumberOfTracks</relatedStateVariable>
            </argument>
            <argument>
               <name>MediaDuration</name> <direction>out</direction>
               <relatedStateVariable>CurrentMediaDuration</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentURI</name> <direction>out</direction>
               <relatedStateVariable>AVTransportURI</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentURIMetaData</name>  <direction>out</direction>
               <relatedStateVariable>AVTransportURIMetaData</relatedStateVariable>
            </argument>
            <argument>
               <name>NextURI</name>  <direction>out</direction>
               <relatedStateVariable>NextAVTransportURI</relatedStateVariable>
            </argument>
            <argument>
               <name>NextURIMetaData</name> <direction>out</direction>
               <relatedStateVariable>NextAVTransportURIMetaData</relatedStateVariable>
            </argument>
            <argument>
               <name>PlayMedium</name> <direction>out</direction>
               <relatedStateVariable>PlaybackStorageMedium</relatedStateVariable>
            </argument>
            <argument>
               <name>RecordMedium</name>  <direction>out</direction>
               <relatedStateVariable>RecordStorageMedium</relatedStateVariable>
            </argument>
            <argument>
               <name>WriteStatus</name> <direction>out</direction>
               <relatedStateVariable>RecordMediumWriteStatus</relatedStateVariable>
            </argument>
         </argumentList>
****************************************************************************/
int GetMediaInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;
  //char *value = NULL;
  char *in_arg;
  std::string result;
  std::string eventArg;
  struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
  //arg = (char *) malloc (1024);
  //eventArg = (char *) malloc (1024);
  //memset(arg,0,1024);
  //memset(eventArg,0,1024);
  in_obj = json_object_new_object();
  if (!in_obj) {
  	render_track();
  	( *out ) = NULL;
  	( *errorString ) = "Internal Error";
  	return UPNP_E_INTERNAL_ERROR;
  }
  //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "dinglei1\n");
  //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%s \n",in);
  in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
  if (!in_arg) {
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
  } else {
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);
  	json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
  }
  value_free(in_arg);
  eventArg = json_object_to_json_string(in_obj);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", eventArg.c_str());
  if(DMREvent_Callback) {
    DMREvent_Callback(DLNA_EVENT_DMR_GETMEDIAINFO,eventArg.c_str(), &result);
  }
  json_object_put(in_obj);
  in_obj = NULL;
  result_obj = json_tokener_parse(result.c_str());
  if (result_obj) {
  	new_obj = json_object_object_get(result_obj, "NrTracks");
  	if (new_obj) {
  		value = (char*)json_object_get_string(new_obj);
  		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "NrTracks =%s  \n",value);
  	} else {
  		value = "0";
  	}
  } else {
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");
  	return UPNP_E_INTERNAL_ERROR;
  }
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "NumberOfTracks", &value);
  if (UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "NrTracks", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "MediaDuration");
  if (new_obj) {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "MediaDuration =%s  \n",value);
  } else {
  	value = "00:00:00";
  }

  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentMediaDuration", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "MediaDuration", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "CurrentURI");
  if(new_obj)
  {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentURI =%s  \n",value);
  }
  //value_free(value);
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURI", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "CurrentURI", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "CurrentURIMetaData");
  if(new_obj)
  {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentURIMetaData =%s  \n",value);
  }
  //value_free(value);
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURIMetaData", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "CurrentURIMetaData", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "NextURI");
  if (new_obj) {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "NextURI =%s  \n",value);
  }
  //value_free(value);
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "NextAVTransportURI", &value);
  if (UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "NextURI", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "NextURIMetaData");
  if (new_obj) {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "NextURIMetaData =%s  \n",value);
  }
  //value_free(value);
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "NextAVTransportURIMetaData", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "NextURIMetaData", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "PlayMedium");
  if(new_obj) {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "PlayMedium =%s  \n",value);
  } else {
  	value = "UNKNOWN";
  }
  //value_free(value);
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "PlaybackStorageMedium", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "PlayMedium", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "RecordMedium");
  if (new_obj) {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "RecordMedium =%s  \n",value);
  } else {
  	value = "UNKNOWN";
  }
  //value_free(value);
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "RecordStorageMedium", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "RecordMedium", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
  new_obj = json_object_object_get(result_obj, "WriteStatus");
  if (new_obj) {
  	value = (char*)json_object_get_string(new_obj);
  	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "WriteStatus =%s  \n",value);
  } else {
  	value = "UNKNOWN";
  }
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "RecordMediumWriteStatus", &value);
  if( UpnpAddToActionResponse( out, "GetMediaInfo",
      render_service_type[AVTRANSPORT_SERVICE], "WriteStatus", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }

  json_object_put(result_obj);
  return UPNP_E_SUCCESS;
}

/***********************************************************************
      <action>
         <name>GetPositionInfo</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>Track</name>  <direction>out</direction>
               <relatedStateVariable>CurrentTrack</relatedStateVariable>
            </argument>
            <argument>
               <name>TrackDuration</name> <direction>out</direction>
               <relatedStateVariable>CurrentTrackDuration</relatedStateVariable>
            </argument>
            <argument>
               <name>TrackMetaData</name> <direction>out</direction>
               <relatedStateVariable>CurrentTrackMetaData</relatedStateVariable>
            </argument>
            <argument>
               <name>TrackURI</name> <direction>out</direction>
               <relatedStateVariable>CurrentTrackURI</relatedStateVariable>
            </argument>
            <argument>
               <name>RelTime</name> <direction>out</direction>
               <relatedStateVariable>RelativeTimePosition</relatedStateVariable>
            </argument>
            <argument>
               <name>AbsTime</name> <direction>out</direction>
               <relatedStateVariable>AbsoluteTimePosition</relatedStateVariable>
            </argument>
            <argument>
               <name>RelCount</name> <direction>out</direction>
               <relatedStateVariable>RelativeCounterPosition</relatedStateVariable>
            </argument>
            <argument>
               <name>AbsCount</name> <direction>out</direction>
               <relatedStateVariable>AbsoluteCounterPosition</relatedStateVariable>
            </argument>
         </argumentList>
      </action>
***********************************************************************/
int GetPositionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *value = NULL;
  char* value_need_free = NULL;
#if 1
	render_track();
	char *in_arg = NULL;
	std::string result;
	std::string eventArg;
	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
	int ret = 0;
	int time = 0;
	char time_str[128] = {0};
	in_obj = json_object_new_object();
	if (!in_obj) {
		render_track();
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
    json_object_put(in_obj);
    ( *out ) = NULL;
		( *errorString ) = "Internal Error";

		return UPNP_E_INTERNAL_ERROR;
 	}	else {
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);

  eventArg = json_object_to_json_string(in_obj);
	if (DMREvent_Callback) {
			DMREvent_Callback(DLNA_EVENT_DMR_GETPOSITIONINFO,eventArg.c_str(), &result);
	}
 	value_free(in_arg);
  json_object_put(in_obj);
  in_obj = NULL;
	result_obj = json_tokener_parse(result.c_str());
	if (result_obj) {
		new_obj = json_object_object_get(result_obj, "Track");
		if (new_obj) {
			value = (char*)json_object_get_string(new_obj);
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Track =%s  \n",value);
		} else {
			value = "0";
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Track =%s  \n",value);
		}
	} else {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");
		render_track();
		json_object_put(result_obj);
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

		return UPNP_E_INTERNAL_ERROR;
	}
#endif
  if( UpnpAddToActionResponse( out, "GetPositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "Track", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
		render_track();
		json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }
	value = NULL;

	new_obj = json_object_object_get(result_obj, "returncode");
	if(new_obj) {
		value = (char*)json_object_get_string(new_obj);
		//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "returncode =%s  \n",value);
		if (value) {
			ret = atoi(value);
      value = NULL;
		}
	} else {
		render_track();
		json_object_put(result_obj);
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}
	//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "ret = %d\n",ret);
	new_obj = json_object_object_get(result_obj, "Duration");
	if (new_obj) {
		value = (char*)json_object_get_string(new_obj);
		if (value) {
			time = atoi(value);
			int h,m,s;
			h = time/3600;
			m = (time - h*3600)/60;
			s = (time- h*3600-m*60);
			sprintf(time_str,"%02d:%02d:%02d",h,m,s);
			h %= 24;
  		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Duration =%s  \n",time_str);
      value = NULL;
		}
		//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "TrackDuration =%s  \n",value);
	}	else {
		value = "00:00:00";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "TrackDuration =%s  \n",value);
	}
	//upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackDuration", &value);
  if( UpnpAddToActionResponse( out, "GetPositionInfo",
    render_service_type[AVTRANSPORT_SERVICE], "TrackDuration",  time_str) != UPNP_E_SUCCESS ) {

    ( *errorString ) = "Internal Error";
		render_track();
		json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
  }

	new_obj = json_object_object_get(result_obj, "TrackMetaData");
	if (new_obj) {
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "TrackMetaData =%s  \n",value);
	} else {
		upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackMetaData", &value_need_free);
		value = value_need_free;
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "TrackMetaData =%s  \n",value);
	}
	//upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackMetaData", &value);
  if( UpnpAddToActionResponse( out, "GetPositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "TrackMetaData", value ) != UPNP_E_SUCCESS ) {
    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    result_obj = NULL;
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value_need_free);
	new_obj = json_object_object_get(result_obj, "TrackURI");
	if (new_obj)	{
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "TrackURI =%s  \n",value);
	}	else {
		upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackURI", &value_need_free);
		value = value_need_free;
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "TrackURI =%s  \n",value);
	}
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackURI", &value);
  if( UpnpAddToActionResponse( out, "GetPositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "TrackURI",  value) != UPNP_E_SUCCESS ) {
    ( *errorString ) = "Internal Error";
		value_free(value);
		render_track();
		json_object_put(result_obj);
    result_obj = NULL;
    return UPNP_E_INTERNAL_ERROR;
  }
	value_free(value_need_free);
	new_obj = json_object_object_get(result_obj, "Position");
	//new_obj = json_object_object_get(result_obj, "RelTime");
	if (new_obj) {
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "RelTime =%s  \n",value);
		if (value)	{
			time = atoi(value);
			int h,m,s;
			h = time/3600;
			m = (time - h*3600)/60;
			s = (time- h*3600-m*60);
			sprintf(time_str,"%02d:%02d:%02d",h,m,s);
  		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "RelTime =%s  \n",time_str);
		}	else {
			sprintf(time_str,"00:00:00");

		}
		//upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "RelativeTimePosition", &value);
    if (UpnpAddToActionResponse( out, "GetPositionInfo",
        render_service_type[AVTRANSPORT_SERVICE], "RelTime", time_str ) != UPNP_E_SUCCESS ) {

      ( *errorString ) = "Internal Error";
      //value_free(value);
      render_track();
      json_object_put(result_obj);
      result_obj = NULL;
      return UPNP_E_INTERNAL_ERROR;
    }
	}
	new_obj = json_object_object_get(result_obj, "Position");
	//new_obj = json_object_object_get(result_obj, "AbsTime");
	if (new_obj) {
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "AbsTime =%s  \n",value);
		if (value) {
			time = atoi(value);
			int h,m,s;
			h = time/3600;
			m = (time - h*3600)/60;
			s = (time - h*3600-m*60);
			sprintf(time_str,"%02d:%02d:%02d",h,m,s);
  		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "AbsTime =%s  \n",time_str);
		} else {
			sprintf(time_str,"00:00:00");
		}
    if( UpnpAddToActionResponse( out, "GetPositionInfo",
        render_service_type[AVTRANSPORT_SERVICE], "AbsTime",  time_str) != UPNP_E_SUCCESS ) {

      ( *errorString ) = "Internal Error";
      //value_free(value);
      render_track();
      json_object_put(result_obj);
      result_obj = NULL;
      return UPNP_E_INTERNAL_ERROR;
    }
	}

	new_obj = json_object_object_get(result_obj, "RelCount");
	if (new_obj) {
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "RelCount =%s  \n",value);
	}	else {
		value = "0";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "RelCount =%s  \n",value);
	}
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "RelativeCounterPosition", &value);
  if( UpnpAddToActionResponse( out, "GetPositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "RelCount", value ) != UPNP_E_SUCCESS ) {

    ( *errorString ) = "Internal Error";
    //value_free(value);
    render_track();
    json_object_put(result_obj);
    result_obj = NULL;
    return UPNP_E_INTERNAL_ERROR;
  }

	new_obj = json_object_object_get(result_obj, "AbsCount");
	if (new_obj) {
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "AbsCount =%s  \n",value);
	} else {
		value ="0";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "AbsCount =%s  \n",value);
	}
  //upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "AbsoluteCounterPosition", &value);
  if( UpnpAddToActionResponse( out, "GetPositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "AbsCount",  value) != UPNP_E_SUCCESS ) {
    ( *errorString ) = "Internal Error";
    json_object_put(result_obj);
    result_obj = NULL;
    return UPNP_E_INTERNAL_ERROR;
  }
  render_track();
  json_object_put(result_obj);
  result_obj = NULL;
  return ret;
}

/************************************************************************
         <name>GetTransportInfo</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentTransportState</name> <direction>out</direction>
               <relatedStateVariable>TransportState</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentTransportStatus</name> <direction>out</direction>
               <relatedStateVariable>TransportStatus</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentSpeed</name> <direction>out</direction>
               <relatedStateVariable>TransportPlaySpeed</relatedStateVariable>
            </argument>
         </argumentList>
*************************************************************************/
int GetTransportInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;
#if 1
	render_track();
	//char *value = NULL;
	char *in_arg;
	//char *arg;
	//char *eventArg;
	std::string result;
	std::string eventArg;


	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
	//arg = (char *) malloc (1024);	
	//eventArg = (char *) malloc (1024);
	//memset(arg,0,1024);
	//memset(eventArg,0,1024);
	in_obj = json_object_new_object();
	if(!in_obj)
	{
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		json_object_put(new_obj);
		json_object_put(in_obj);
		//value_free(arg);
		//value_free(eventArg);		
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

		return UPNP_E_INTERNAL_ERROR;
	}	
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "dinglei1\n");
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%s \n",in);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if(!in_arg)
	{
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
	}
	else
	{
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);
	value_free(in_arg);
	//strcpy(eventArg,json_object_to_json_string(in_obj));
	  eventArg = json_object_to_json_string(in_obj);
	if (DMREvent_Callback)
	{	
			DMREvent_Callback(DLNA_EVENT_DMR_GETTRANSPORTINFO,eventArg.c_str(), &result);
	}
	result_obj = json_tokener_parse(result.c_str());
	//result_obj = json_tokener_parse(arg);
	if(result_obj)
	{
		new_obj = json_object_object_get(result_obj, "CurrentTransportState");
		if(new_obj)
		{
			value = (char*)json_object_get_string(new_obj);
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentTransportState =%s  \n",value);
		}
	}
	else
	{	//value = "00";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");		
		render_track();
		json_object_put(result_obj);
		json_object_put(new_obj);
		json_object_put(in_obj);
		//value_free(arg);
		//value_free(eventArg);
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

		return UPNP_E_INTERNAL_ERROR;
	}
	 if( UpnpAddToActionResponse( out, "GetTransportInfo", render_service_type[AVTRANSPORT_SERVICE], "CurrentTransportState", value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		json_object_put(new_obj);
		json_object_put(in_obj);
		//value_free(arg);
		//value_free(eventArg);
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

            return UPNP_E_INTERNAL_ERROR;
        }

	new_obj = json_object_object_get(result_obj, "CurrentTransportStatus");
	if(new_obj)
	{
		value = (char*)json_object_get_string(new_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentTransportStatus =%s  \n",value);
	}
	 if( UpnpAddToActionResponse( out, "GetTransportInfo", render_service_type[AVTRANSPORT_SERVICE], "CurrentTransportStatus", value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		json_object_put(new_obj);
		json_object_put(in_obj);
		//value_free(arg);
		//value_free(eventArg);
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

            return UPNP_E_INTERNAL_ERROR;
        }

	 if(result_obj)
	{
		new_obj = json_object_object_get(result_obj, "CurrentSpeed");
		if(new_obj)
		{
			value = (char*)json_object_get_string(new_obj);
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentSpeed =%s  \n",value);
		}
	}
	else
	{	//value = "00";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");	
		render_track();
		json_object_put(result_obj);
		json_object_put(new_obj);
		json_object_put(in_obj);
		//value_free(arg);
		//value_free(eventArg);
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

		return UPNP_E_INTERNAL_ERROR;
	}
	 if( UpnpAddToActionResponse( out, "GetTransportInfo", render_service_type[AVTRANSPORT_SERVICE], "CurrentSpeed", value ) != UPNP_E_SUCCESS ) {
            ( *out ) = NULL;
            ( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		json_object_put(new_obj);
		json_object_put(in_obj);
		//value_free(arg);
		//value_free(eventArg);
		( *out ) = NULL;
		( *errorString ) = "Internal Error";

            return UPNP_E_INTERNAL_ERROR;
        }
	 	//render_track();
		json_object_put(result_obj);
	//render_track();
		json_object_put(new_obj);
	//render_track();
		json_object_put(in_obj);
	//		render_track();

		//value_free(arg);
		//value_free(eventArg);
#else
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", &value);
  if( UpnpAddToActionResponse( out, "GetTransportInfo",
      render_service_type[AVTRANSPORT_SERVICE], "CurrentTransportState", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportStatus", &value);
  if( UpnpAddToActionResponse( out, "GetTransportInfo",
      render_service_type[AVTRANSPORT_SERVICE], "CurrentTransportStatus", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportPlaySpeed", &value);
  if( UpnpAddToActionResponse( out, "GetTransportInfo",
      render_service_type[AVTRANSPORT_SERVICE], "CurrentSpeed", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  //
#endif
  return UPNP_E_SUCCESS;
}

/***************************************************************************
         <name>GetTransportSettings</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>PlayMode</name> <direction>out</direction>
               <relatedStateVariable>CurrentPlayMode</relatedStateVariable>
            </argument>
            <argument>
               <name>RecQualityMode</name> <direction>out</direction>
               <relatedStateVariable>CurrentRecordQualityMode</relatedStateVariable>
            </argument>
         </argumentList>
*****************************************************************************/
int GetTransportSettings( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentPlayMode", &value);
  if ( UpnpAddToActionResponse( out, "GetTransportSettings",
      render_service_type[AVTRANSPORT_SERVICE], "PlayMode", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentRecordQualityMode", &value);
  if ( UpnpAddToActionResponse( out, "GetTransportSettings",
      render_service_type[AVTRANSPORT_SERVICE], "RecQualityMode", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  return UPNP_E_SUCCESS;
}

/*************************************************************************
         <name>Next</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
         </argumentList>
 **************************************************************************/
int Player_Next( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  //	int index;

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "NumberOfTracks", &value);
  if (!value){
    ( *out ) = NULL;
    ( *errorString ) = "NumberOfTracks is NULL";
    return UPNP_E_INTERNAL_ERROR;
  }
  if (strstr(value, "1")){
    ( *out ) = NULL;
    ( *errorString ) = "Illegal seek target";
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "only 1 track, can not change to next\n");
    return 711;
  } else {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "multi track of media is not supported now\n");
  }
  value_free(value);
  if (UpnpAddToActionResponse( out, "Next",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}

int media_play(char *speed)
{
  int ret = -1;
  char *url = NULL;
  char *t_state = NULL;

  std::string result;
  std::string eventArg;
  struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
  char *instanceID = NULL;
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURI", &url);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media_play: AVTransportURI = %s\n", url);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "InstanceID", &instanceID);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media_play: instanceID = %s\n", instanceID);

  if (url) {
    upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", &t_state);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media_play: t_state = (%s) .\n", t_state);
    render_track();

    in_obj = json_object_new_object();
    if (!in_obj) {
      render_track();
      return UPNP_E_INTERNAL_ERROR;
    }
    json_object_object_add(in_obj, "instanceID", json_object_new_string(instanceID));
    json_object_object_add(in_obj, "PlayUrl", json_object_new_string(url));
    switch(property.media_type){
    case  M_PICTURE:
      json_object_object_add(in_obj, "mediaType", json_object_new_string("PICTURE"));
      break;
    case  M_AUDIO:
      json_object_object_add(in_obj, "mediaType", json_object_new_string("M_AUDIO"));
      break;
    case  M_VIDEO:
      json_object_object_add(in_obj, "mediaType", json_object_new_string("M_VIDEO"));
      break;
    default:
      break;
    }
    if (property.dc_title){
      struct json_object* trackMetadata = json_object_new_object();
      json_object_object_add(trackMetadata, "dc_title", json_object_new_string(property.dc_title));

      json_object_object_add(in_obj, "TrackMetadata", trackMetadata);
    }
    json_object_object_add(in_obj, "speed", json_object_new_string(speed));
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "PlayUrl = %s\n", url);
    eventArg = json_object_to_json_string(in_obj);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg.c_str());
    if (DMREvent_Callback) {
      DMREvent_Callback(DLNA_EVENT_DMR_PLAY,eventArg.c_str(), &result);
    }

    json_object_put(in_obj);
    in_obj = NULL;
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "arg =%s\n",result.c_str());
    char *value = NULL;
    result_obj = json_tokener_parse(result.c_str());
    if(result_obj) {
      new_obj = json_object_object_get(result_obj, "returncode");
      if(new_obj) {
        value = (char*)json_object_get_string(new_obj);
        if(value) {
          HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "returncode =%s  \n",value);
          ret = atoi(value);
        }
      }
    } else {	//value = "00";
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");
      json_object_put(result_obj);
      result_obj = NULL;
      return UPNP_E_INTERNAL_ERROR;
    }
    value_free(t_state);
    value_free(url);
  }
  json_object_put(result_obj);
  result_obj = NULL;
  value_free(instanceID);
  return ret;
}

int media_stop(void)
{
  render_track();
  std::string result;
  std::string eventArg;
  struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
  char *instanceID = NULL;
  //ithread_mutex_lock(&var_mux);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "InstanceID", &instanceID);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media_stop: instanceID = %s\n", instanceID);

  in_obj = json_object_new_object();
  if (!in_obj) {
    //value_free(value);
    render_track();
    return UPNP_E_INTERNAL_ERROR;
  }
  json_object_object_add(in_obj, "instanceID", json_object_new_string(instanceID));

  eventArg = json_object_to_json_string(in_obj);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg.c_str());
  if (DMREvent_Callback) {
    DMREvent_Callback(DLNA_EVENT_DMR_STOP,eventArg.c_str(), &result);
  }
  json_object_put(in_obj);
  in_obj = NULL;
  value_free(instanceID);
  //ithread_mutex_unlock(&var_mux);
  return 0;
}

int media_pause(void)
{
  render_track();
  int ret = 0;

  std::string result;
  std::string eventArg;
  struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
  char *instanceID = NULL;
  //ithread_mutex_lock(&var_mux);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "InstanceID", &instanceID);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media_pause: instanceID = %s\n", instanceID);

  in_obj = json_object_new_object();
  if(!in_obj) {
    //value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  json_object_object_add(in_obj, "instanceID", json_object_new_string(instanceID));

  eventArg = json_object_to_json_string(in_obj);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg.c_str());
  if(DMREvent_Callback) {
    DMREvent_Callback(DLNA_EVENT_DMR_PAUSE,eventArg.c_str(), &result);
  }
  json_object_put(in_obj);
  in_obj = NULL;
  value_free(instanceID);
  //ithread_mutex_unlock(&var_mux);

  return ret;
}

int media_fast_forward(int level)
{
  char *value = NULL;
  //	int index;
  int ret = 0;

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", &value);
  if(!value){
    return -1;
  }
  if(!strstr(value, "PLAYING") || property.media_type == M_PICTURE){
    ret =  -1;
    goto error;
  }
#ifdef NODEFINE_DINGLEI
  DMR_AVPlayer_Forward_dinglei(&g_dmr_player, level);
#endif
  //HySDK_VPlayer_Forward(level);
  //ymm_stream_playerSetTrickMode(0, YX_FAST_FORWARD, level);
error:
  value_free(value);
  return ret;
}

int media_fast_rewind(int level)
{
  char *value = NULL;
  //	int index;
  int ret = 0;

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", &value);
  if(!value){
    return -1;
  }
  if(!strstr(value, "PLAYING") || property.media_type == M_PICTURE){
    ret =  -1;
    goto error;
  }
#ifdef NODEFINE_DINGLEI
  DMR_AVPlayer_Reverse_dinglei(&g_dmr_player, level);
#endif
  //HySDK_VPlayer_Reverse(level);
  //ymm_stream_playerSetTrickMode(0, YX_FAST_REW, level);
error:
  value_free(value);
  return ret;
}

int media_seek(long long  target)
{
  //	upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "A_ARG_TYPE_SeekMode", seek_mode);
  //	upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "A_ARG_TYPE_SeekTarget", seek_target);
  render_track();
  //char *in_arg;
  std::string result;
  std::string eventArg;
  struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
  char *instanceID = NULL;
  char *seek_mode = NULL;
  char *seek_target = NULL;

  in_obj = json_object_new_object();
  if (!in_obj) {
    render_track();
    return UPNP_E_INTERNAL_ERROR;
  }
  //ithread_mutex_lock(&var_mux);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "InstanceID", &instanceID);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media_play: instanceID = %s\n", instanceID);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "A_ARG_TYPE_SeekMode", &seek_mode);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "seek_mode = %s\n", seek_mode);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "A_ARG_TYPE_SeekTarget", &seek_target);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "seek_target= %s\n", seek_target);

  json_object_object_add(in_obj, "instanceID", json_object_new_string(instanceID));
  json_object_object_add(in_obj, "seek_mode", json_object_new_string(seek_mode));
  json_object_object_add(in_obj, "seek_target", json_object_new_string(seek_target));
  eventArg = json_object_to_json_string(in_obj);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg.c_str());
  if (DMREvent_Callback) {
    DMREvent_Callback(DLNA_EVENT_DMR_SEEK,eventArg.c_str(), &result);
  }
  json_object_put(in_obj);
  in_obj = NULL;
  value_free(instanceID);
  value_free(seek_mode);
  value_free(seek_target);
  //ithread_mutex_unlock(&var_mux);
  return 0;
}


/*************************************************************************
  <name>Pause</name>
  <argumentList>
  <argument>
  <name>InstanceID</name> <direction>in</direction>
  <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
  </argument>
  </argumentList>
 ***************************************************************************/
int Player_Pause( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  int index = -1;
  render_track();

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTransportActions", &value);
  if(!value){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    render_track();
    return UPNP_E_INTERNAL_ERROR;
  }
  /*if(!strstr(value, "Pause")){
    ( *out ) = NULL;
    ( *errorString ) = "Invalid Actions";
    render_track();

    return 401;
    } else {*/
  if(media_pause() != 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  render_player_state(3);
  //}
  value_free(value);
  if (UpnpAddToActionResponse( out, "Pause",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  std::string buf;
  std::string dst_buf;
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "PAUSED_PLAYBACK");
  if (index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }

  av_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "LastChange", dst_buf.c_str());
  render_track();

  UpnpNotify( render_handle,
    upnp_service_table[AVTRANSPORT_SERVICE].service_udn,
    upnp_service_table[AVTRANSPORT_SERVICE].service_id,
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_name[index],
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_value[index], 1 );

  render_track();

  return UPNP_E_SUCCESS;
}

/*************************************************************************
  <name>Play</name>
  <argumentList>
  <argument>
  <name>InstanceID</name> <direction>in</direction>
  <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
  </argument>
  <argument>
  <name>Speed</name> <direction>in</direction>
  <relatedStateVariable>TransportPlaySpeed</relatedStateVariable>
  </argument>
  </argumentList>
 ***************************************************************************/
int Player_Play( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  int index = -1;
  int ret = 0;
  render_track();
#ifdef NODEFINE_DINGLEI
  if(render_connection_income(15000) != 0){
    ( *out ) = NULL;
    ( *errorString ) = "connection refused";
    return UPNP_E_CANCELED;
  }
#endif
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "media type is %d\n", property.media_type);
  value = UpnpUtil_GetFirstDocumentItem( in, "Speed" );
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Speed is %s\n", value);

  ret = media_play(value);
  value_free(value);
  render_player_state(1);
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "PLAYING");
  if(index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  render_track();
  if( UpnpAddToActionResponse( out, "Play",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  render_track();
  std::string buf;
  std::string dst_buf;
  av_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "LastChange", dst_buf.c_str());
  render_track();

  UpnpNotify( render_handle,
    upnp_service_table[AVTRANSPORT_SERVICE].service_udn,
    upnp_service_table[AVTRANSPORT_SERVICE].service_id,
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_name[index],
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_value[index], 1 );

  render_track();

  return ret;
}

/**************************************************************************
	<name>Previous</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>
               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
         </argumentList>
***************************************************************************/
int Player_Previous( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  //	int index;

  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "NumberOfTracks", &value);
  if(!value){
    ( *out ) = NULL;
    ( *errorString ) = "NumberOfTracks is NULL";
    return UPNP_E_INTERNAL_ERROR;
  }
  if(strstr(value, "1")){
    ( *out ) = NULL;
    ( *errorString ) = "Illegal seek target";
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "only 1 track, can not change to previous\n");
    value_free(value);
    return 711;
  } else {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "multi track of media is not supported now\n");
  }
  value_free(value);
  if (UpnpAddToActionResponse( out, "Previous",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}


/**************************************************************************
         <name>Seek</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>Unit</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_SeekMode</relatedStateVariable>
            </argument>
            <argument>
               <name>Target</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_SeekTarget</relatedStateVariable>
            </argument>
         </argumentList>
*****************************************************************************/
extern int http_head_cmd_usr_define( IN char *url, IN char *usr_str, OUT char *response_buf, IN int response_buf_len, OUT int * response_len);
int Player_Seek( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *seek_mode = NULL;
  char *seek_target = NULL;
  long long  target = 0;
  //	char *url = NULL;
  int ret = 0;
  int index = -1;
  std::string buf;
  std::string dst_buf;
  render_track();

  seek_mode = UpnpUtil_GetFirstDocumentItem( in, "Unit" );
  if(!seek_mode)
    goto error;
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "seek_mode is %s\n", seek_mode);
  /*if(strcmp("X_DLNA_REL_BYTE", seek_mode)){
    value_free(seek_mode);
    ( *out ) = NULL;
    ( *errorString ) = "Seek mode not supported";
    return 710;
    }*/
  seek_target = UpnpUtil_GetFirstDocumentItem( in, "Target" );
  if(!seek_target)
    goto error;

  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "seek_target is %s\n", seek_target);
#if defined(OS_WIN32)
  target = atol(seek_target);
#else
  target = atoll(seek_target);
#endif
  upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "A_ARG_TYPE_SeekMode", seek_mode);
  upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "A_ARG_TYPE_SeekTarget", seek_target);
  value_free(seek_target);
  value_free(seek_mode);
  ret = media_seek(target);
  if(ret != 0){
    ( *out ) = NULL;
    ( *errorString ) = "Error";
    return ret;
  }
  if( UpnpAddToActionResponse( out, "Seek",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }

  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "PLAYING");
  if(index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }

  av_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "LastChange", dst_buf.c_str());
  render_track();

  UpnpNotify( render_handle,
    upnp_service_table[AVTRANSPORT_SERVICE].service_udn,
    upnp_service_table[AVTRANSPORT_SERVICE].service_id,
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_name[index],
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_value[index], 1 );

  render_track();

  return UPNP_E_SUCCESS;
error:
  ( *out ) = NULL;
  ( *errorString ) = "Internal Error";
  return UPNP_E_INTERNAL_ERROR;
}

/******************************************************************************
  	    <name>X_DLNA_GetBytePositionInfo</name>
		<argumentList>
	    <argument>
	      <name>InstanceID</name>
	      <direction>in</direction>
	      <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
	    </argument>
	    <argument>
  	    <name>TrackSize</name>
	      <direction>out</direction>
			<relatedStateVariable>X_DLNA_CurrentTrackSize</relatedStateVariable>
		</argument>
	    <argument>
	      <name>RelByte</name>
	      <direction>out</direction>
	      <relatedStateVariable>X_DLNA_RelativeBytePosition</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>AbsByte</name>
	      <direction>out</direction>
	      <relatedStateVariable>X_DLNA_AbsoluteBytePosition</relatedStateVariable>
	    </argument>
		</argumentList>
*******************************************************************************/

int X_DLNA_GetBytePositionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  //int index = -1;
  //void *handle;
  long long  offset = 0;
  long long  len= 0;
  char str_off[21] = {0};
  char str_len[21] = {0};
  char *id = NULL;
  char *t_state = NULL;

  render_track();

  id = UpnpUtil_GetFirstDocumentItem( in, "InstanceID" );
  if(!id){
    ( *out ) = NULL;
    ( *errorString ) = "InstanceID is NULL";
    return UPNP_SOAP_E_INVALID_ARGS;
  }
  if(strcmp(id, "0")){
    ( *out ) = NULL;
    ( *errorString ) = "InstanceID is NULL";
    value_free(id);
    return UPNP_SOAP_E_INVALID_ID;
  }
  value_free(id);
  upnp_var_get(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", &t_state);
  if(t_state){
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "value is %s, media type is %d\n", t_state, property.media_type);
    if( property.media_type == M_PICTURE){
      value_free(t_state);
      ( *out ) = NULL;
      ( *errorString ) = "X_DLNA_GetBytePositionInfo mode not supported";
      return UPNP_E_INTERNAL_ERROR;
    }
  }
  if(!strstr(t_state, "STOPPED") && !strstr(t_state, "NO_MEDIA_PRESENT")){
    //handle = ymm_stream_handle_get(0);
    //ymm_stream_getOffset(handle, &offset);
    //ymm_stream_getLength(handle, &len);
    //offset = HySDK_VPlayer_GetPositionLength();
    //len = HySDK_VPlayer_GetCurrentPosition();
#ifdef NODEFINE_DINGLEI
    offset	= DMR_AVPlayer_GetTotalLength_dinglei(&g_dmr_player);
    len		= DMR_AVPlayer_GetCurrentLength_dinglei(&g_dmr_player);
#endif
    sprintf(str_off, "%lld", offset);
    sprintf(str_len, "%lld", len);
  }
  value_free(t_state);
  render_track();
  render_msg("offset is %lld, len is %lld\n", offset, len);
  if( UpnpAddToActionResponse( out, "X_DLNA_GetBytePositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "TrackSize", str_len ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  if( UpnpAddToActionResponse( out, "X_DLNA_GetBytePositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "RelByte", str_off ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  if( UpnpAddToActionResponse( out, "X_DLNA_GetBytePositionInfo",
      render_service_type[AVTRANSPORT_SERVICE], "AbsByte", str_off ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}

/******************************************************************************
	    <argument>
	      <name>RemoteProtocolInfo</name>	      <direction>in</direction>
	      <relatedStateVariable>A_ARG_TYPE_ProtocolInfo</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>PeerConnectionManager</name>	      <direction>in</direction>
	      <relatedStateVariable>A_ARG_TYPE_ConnectionManager</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>PeerConnectionID</name>	      <direction>in</direction>
	      <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>Direction</name>	      <direction>in</direction>
	      <relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>ConnectionID</name>	      <direction>out</direction>
	      <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>AVTransportID</name>	      <direction>out</direction>
	      <relatedStateVariable>A_ARG_TYPE_AVTransportID</relatedStateVariable>
	    </argument>
	    <argument>
	      <name>RcsID</name>	      <direction>out</direction>
	      <relatedStateVariable>A_ARG_TYPE_RcsID</relatedStateVariable>
	    </argument>

******************************************************************************/
int PrepareForConnection( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	char *value = NULL;

	render_track();
	value = UpnpUtil_GetFirstDocumentItem( in, "RemoteProtocolInfo" );
	//media_stop();
	if(value){
		media_propertity_clear(&property);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "RemoteProtocolInfo is \n%s\n", value);
		upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ProtocolInfo", value);
		property.m_detail.protocolInfo = strdup(value);
		if(strstr(value, "http-get:*:video")){
			property.media_type = M_VIDEO;
		} else if (strstr(value, "http-get:*:audio")){
			property.media_type = M_AUDIO;
		} else if (strstr(value, "http-get:*:image")){
			property.media_type = M_PICTURE;
		} else if(strstr(value, "http-get:*:application/x-dtcp1")){
			if(strstr(value, "CONTENTFORMAT=video")){
				render_msg("dtcp video\n");
				property.media_type = M_VIDEO;
			} else if(strstr(value, "CONTENTFORMAT=audio")){
				render_msg("dtcp video\n");
				property.media_type = M_AUDIO;
			}  else if(strstr(value, "CONTENTFORMAT=image")){
				render_msg("dtcp video\n");
				property.media_type = M_PICTURE;
			}
		//http-get:*:application/x-dtcp1;CONTENTFORMAT=video/mpeg:*
		}
		transport_action_set(&property);
		value_free(value);
	}
	value = UpnpUtil_GetFirstDocumentItem( in, "PeerConnectionManager" );
	if(value){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "PeerConnectionManager is \n%s\n", value);
		upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ConnectionManager", value);
		value_free(value);
	}
	value = UpnpUtil_GetFirstDocumentItem( in, "PeerConnectionID" );
	if(value){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "PeerConnectionID is \n%s\n", value);
		upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ConnectionID", value);
		value_free(value);
	}
	value = UpnpUtil_GetFirstDocumentItem( in, "Direction" );
	if(value){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Direction is \n%s\n", value);
		upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_Direction", value);
		value_free(value);
	}

	upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ConnectionID", &value);
	if (UpnpAddToActionResponse( out, "PrepareForConnection",
	   render_service_type[CONNECTIONMANAGER_SERVICE], "ConnectionID", value ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		value_free(value);
		return UPNP_E_INTERNAL_ERROR;
	}
	value_free(value);
	upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_AVTransportID", &value);
	if (UpnpAddToActionResponse( out, "PrepareForConnection",
	   render_service_type[CONNECTIONMANAGER_SERVICE], "AVTransportID", value ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		value_free(value);
		return UPNP_E_INTERNAL_ERROR;
	}
	value_free(value);
	upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_RcsID", &value);
	if (UpnpAddToActionResponse( out, "PrepareForConnection",
	   render_service_type[CONNECTIONMANAGER_SERVICE], "RcsID", value ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		value_free(value);
		return UPNP_E_INTERNAL_ERROR;
	}
	value_free(value);
  return UPNP_E_SUCCESS;
}

/****************************************************************************
       <name>ConnectionComplete</name>
		<argument>
	      	<name>ConnectionID</name>	   		  <direction>in</direction>
		      <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>
		</argument>
****************************************************************************/
int ConnectionComplete( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
//STOPPED,PAUSED_PLAYBACK,PAUSED_RECORDING,PLAYING,RECORDING,TRANSITIONING,NO_MEDIA_PRESENT

	render_track();
	media_stop();
	upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "NO_MEDIA_PRESENT");
	upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ProtocolInfo", 0);
	if (UpnpAddToActionResponse( out, "ConnectionComplete",
      render_service_type[CONNECTIONMANAGER_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
	}
  return UPNP_E_SUCCESS;
}

/****************************************************************************
         <name>SetAVTransportURI</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentURI</name> <direction>in</direction>
               <relatedStateVariable>AVTransportURI</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentURIMetaData</name> <direction>in</direction>
               <relatedStateVariable>AVTransportURIMetaData</relatedStateVariable>
            </argument>
         </argumentList>
*****************************************************************************/
int SetAVTransportURI( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  int index = -1;
  std::string buf;
  std::string dst_buf;
  render_track();
#ifdef NODEFINE_DINGLEI
  media_stop();
#endif
  value = UpnpUtil_GetFirstDocumentItem( in, "InstanceID" );
  if (value)	{
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID is %s\n", value);
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "InstanceID", value);
    value_free(value);
    value = UpnpUtil_GetFirstDocumentItem( in, "CurrentURI" );
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "uri is %s\n", value);
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURI", value);
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackURI", value);
  } else {
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURI", "");
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackURI", "");
  }
  if (index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  if(value){
    render_track();
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "STOPPED");
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "NumberOfTracks", "1");
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrack", "1");
    value_free(value);
  } else {
    render_track();
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "NO_MEDIA_PRESENT");
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "NumberOfTracks", "0");
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrack", "0");
    media_propertity_clear(&property);
    transport_action_set(&property);
    goto end;
  }
  render_track();
  value_free(value);
  render_track();
  if (index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  render_track();
  value = UpnpUtil_GetFirstDocumentItem( in, "CurrentURIMetaData" );
  if (value)	{
    render_track();
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "uri is %s\n", value);

    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURIMetaData", value);
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackMetaData", value);
    media_propertity_clear(&property);
    media_propertity_get(value, &property);
    transport_action_set(&property);
    render_track();
    value_free(value);
  } else {
    //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentURIMetaData is NULL\n");
    //upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ProtocolInfo", NULL);
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "AVTransportURIMetaData", "");
    render_track();
    index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentTrackMetaData", "");
    render_track();
  }

end:
  render_track();
  av_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "LastChange", dst_buf.c_str());
  render_track();

  UpnpNotify( render_handle,
    upnp_service_table[AVTRANSPORT_SERVICE].service_udn,
    upnp_service_table[AVTRANSPORT_SERVICE].service_id,
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_name[index],
    ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_value[index], 1 );

  render_track();
  if (UpnpAddToActionResponse( out, "SetAVTransportURI",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    (*out) = NULL;
    (*errorString) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}


/****************************************************************************
      <action>
         <name>SetPlayMode</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>NewPlayMode</name> <direction>in</direction>
               <relatedStateVariable>CurrentPlayMode</relatedStateVariable>
            </argument>
         </argumentList>
      </action>
*****************************************************************************/
int SetPlayMode( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  char *value = NULL;
  int index = -1;

  render_track();
  value = UpnpUtil_GetFirstDocumentItem( in, "NewPlayMode" );
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "CurrentPlayMode", value);
  value_free(value);
  if (index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  UpnpNotify(render_handle, upnp_service_table[AVTRANSPORT_SERVICE].service_udn,
             upnp_service_table[AVTRANSPORT_SERVICE].service_id,
             (const char **)&upnp_service_table[AVTRANSPORT_SERVICE].var_name[index],
             (const char **)&upnp_service_table[AVTRANSPORT_SERVICE].var_value[index], 1 );
  if (UpnpAddToActionResponse( out, "SetPlayMode",
      render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}

/***************************************************************************
         <name>Stop</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
         </argumentList>
****************************************************************************/
int Player_Stop( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	int index = -1;

	media_stop();
	render_player_state(2);
	index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "STOPPED");
	if (index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
	}
  if (UpnpAddToActionResponse( out, "Stop",
     render_service_type[AVTRANSPORT_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  std::string buf;
  std::string dst_buf;
  av_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "LastChange", dst_buf.c_str());
  render_track();

  UpnpNotify( render_handle,
            upnp_service_table[AVTRANSPORT_SERVICE].service_udn,
            upnp_service_table[AVTRANSPORT_SERVICE].service_id,
            ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_name[index],
            ( const char ** )&upnp_service_table[AVTRANSPORT_SERVICE].var_value[index], 1 );

  render_track();

  return UPNP_E_SUCCESS;
}

// 4 connection manager

/************************************************************************
      <action>
         <name>GetCurrentConnectionIDs</name>
         <argumentList>
            <argument>
               <name>ConnectionIDs</name> <direction>out</direction>
               <relatedStateVariable>CurrentConnectionIDs</relatedStateVariable>
            </argument>
         </argumentList>
      </action>
*************************************************************************/
int GetCurrentConnectionIDs( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "CurrentConnectionIDs", &value);
  if (UpnpAddToActionResponse( out, "GetCurrentConnectionIDs",
      render_service_type[CONNECTIONMANAGER_SERVICE], "ConnectionIDs", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  return UPNP_E_SUCCESS;
}


/************************************************************************
      <action>
         <name>GetCurrentConnectionInfo</name>
         <argumentList>
            <argument>
               <name>ConnectionID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>
            </argument>
            <argument>
               <name>RcsID</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_RcsID</relatedStateVariable>
            </argument>
            <argument>
               <name>AVTransportID</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_AVTransportID</relatedStateVariable>
            </argument>
            <argument>
               <name>ProtocolInfo</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_ProtocolInfo</relatedStateVariable>
            </argument>
            <argument>
               <name>PeerConnectionManager</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_ConnectionManager</relatedStateVariable>
            </argument>
            <argument>
               <name>PeerConnectionID</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_ConnectionID</relatedStateVariable>
            </argument>
            <argument>
               <name>Direction</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_Direction</relatedStateVariable>
            </argument>
            <argument>
               <name>Status</name> <direction>out</direction>
               <relatedStateVariable>A_ARG_TYPE_ConnectionStatus</relatedStateVariable>
            </argument>
         </argumentList>
      </action>
*****************************************************************************/
int GetCurrentConnectionInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_RcsID", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "RcsID", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_AVTransportID", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "AVTransportID",  value) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }		render_track();

  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ProtocolInfo", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "ProtocolInfo", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ConnectionManager", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "PeerConnectionManager",  value) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ConnectionID", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "PeerConnectionID", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_Direction", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "Direction",  value) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "A_ARG_TYPE_ConnectionStatus", &value);
  if( UpnpAddToActionResponse( out, "GetCurrentConnectionInfo", render_service_type[CONNECTIONMANAGER_SERVICE], "Status",  value) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

  value_free(value);
  return UPNP_E_SUCCESS;
}


/****************************************************************************
      <action>
         <name>GetProtocolInfo</name>
         <argumentList>
            <argument>
               <name>Source</name> <direction>out</direction>
               <relatedStateVariable>SourceProtocolInfo</relatedStateVariable>
            </argument>
            <argument>
               <name>Sink</name> <direction>out</direction>
               <relatedStateVariable>SinkProtocolInfo</relatedStateVariable>
            </argument>
         </argumentList>
      </action>
*****************************************************************************/
int GetProtocolInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "SourceProtocolInfo", &value);
  render_track();
  if( UpnpAddToActionResponse( out, "GetProtocolInfo",
      render_service_type[CONNECTIONMANAGER_SERVICE], "Source", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  upnp_var_get(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "SinkProtocolInfo", &value);
  if( UpnpAddToActionResponse( out, "GetProtocolInfo",
      render_service_type[CONNECTIONMANAGER_SERVICE], "Sink",  value) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  render_track();
  return UPNP_E_SUCCESS;
}

//Render Control Action
/***********************************************************************
         <name>GetBlueVideoBlackLevel</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentBlueVideoBlackLevel</name> <direction>out</direction>
               <relatedStateVariable>BlueVideoBlackLevel</relatedStateVariable>
            </argument>
         </argumentList>
***********************************************************************/
int GetBlueVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/**********************************************************************
         <name>GetBrightness</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentBrightness</name> <direction>out</direction>
               <relatedStateVariable>Brightness</relatedStateVariable>
            </argument>
         </argumentList>
**********************************************************************/
int GetBrightness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Brightness", &value);
  if( UpnpAddToActionResponse( out, "GetBrightness",
      render_service_type[RENDERINGCONTROL_SERVICE], "CurrentBrightness", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  return UPNP_E_SUCCESS;
}

/**********************************************************************
         <name>GetColorTemperature</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentColorTemperature</name> <direction>out</direction>
               <relatedStateVariable>ColorTemperature</relatedStateVariable>
            </argument>
         </argumentList>
**********************************************************************/
int GetColorTemperature( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/*********************************************************************
         <name>GetContrast</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentContrast</name> <direction>out</direction>
               <relatedStateVariable>Contrast</relatedStateVariable>
            </argument>
         </argumentList>
*********************************************************************/
int GetContrast( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Contrast", &value);
  if( UpnpAddToActionResponse( out, "GetContrast",
     render_service_type[RENDERINGCONTROL_SERVICE], "CurrentContrast", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  return UPNP_E_SUCCESS;
}
/*******************************************************************
	<name>GetGreenVideoBlackLevel</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentGreenVideoBlackLevel</name> <direction>out</direction>
               <relatedStateVariable>GreenVideoBlackLevel</relatedStateVariable>
            </argument>
         </argumentList>
********************************************************************/
int GetGreenVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/**********************************************************************
         <name>GetMute</name>
         <argumentList>
            <argument>
               <name>InstanceID</name> <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>Channel</name>                <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentMute</name>                <direction>out</direction>
               <relatedStateVariable>Mute</relatedStateVariable>
            </argument>
         </argumentList>
************************************************************************/
int GetMute( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *value = NULL;
	char *in_arg = NULL;
	std::string result;
	std::string eventArg;
	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;

	in_obj = json_object_new_object();
	if (!in_obj) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
    value_free(in_arg);
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);

	in_arg = UpnpUtil_GetFirstDocumentItem(in,"Channel");
	if(!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "Channel", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "Channel", json_object_new_string(in_arg));
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Channel = %s\n", in_arg);
		eventArg = json_object_to_json_string(in_obj);
		if (DMREvent_Callback){
			DMREvent_Callback(DLNA_EVENT_DMR_GETMUTE,eventArg.c_str(), &result);
		}
	}
  json_object_put(in_obj);
  in_obj = NULL;
	if( UpnpAddToActionResponse( out, "GetMute",
     render_service_type[RENDERINGCONTROL_SERVICE], "Channel", in_arg ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		value_free(in_arg);
		return UPNP_E_INTERNAL_ERROR;
  }
	value_free(in_arg);

	result_obj = json_tokener_parse( result.c_str());
	if (result_obj) {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "json_object_array_length%d\n",json_object_array_length(result_obj));
		new_obj = json_object_object_get(result_obj, "Mute");
		if (new_obj) {
			value = (char*)json_object_get_string(new_obj);
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "value =%s  \n",value);
		}
	}	else {	//value = "00";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");
	}
	//upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Mute", &value);
	if( UpnpAddToActionResponse( out, "GetMute",
      render_service_type[RENDERINGCONTROL_SERVICE], "CurrentMute", value ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetMute err\n");
		return UPNP_E_INTERNAL_ERROR;
  }
	//value_free(value);
	json_object_put(result_obj);
	render_track();
  return UPNP_E_SUCCESS;
}

/***********************************************************************
         <name>GetRedVideoBlackLevel</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>
               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentRedVideoBlackLevel</name>
               <direction>out</direction>
               <relatedStateVariable>RedVideoBlackLevel</relatedStateVariable>
            </argument>
         </argumentList>
************************************************************************/
int GetRedVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/*************************************************************************
      <action>
         <name>GetSharpness</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentSharpness</name>               <direction>out</direction>
               <relatedStateVariable>Sharpness</relatedStateVariable>
            </argument>
         </argumentList>
****************************************************************************/
int GetSharpness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *value = NULL;

  upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Sharpness", &value);
  if ( UpnpAddToActionResponse( out, "GetSharpness",
     render_service_type[RENDERINGCONTROL_SERVICE], "CurrentSharpness", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  value_free(value);
  return UPNP_E_SUCCESS;
}


/***************************************************************************
	<name>GetVolume</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>Channel</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentVolume</name>               <direction>out</direction>
               <relatedStateVariable>Volume</relatedStateVariable>
            </argument>
         </argumentList>
***************************************************************************/
int GetVolume( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *value = NULL;
	char *in_arg;
	std::string result;
	std::string eventArg;
	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;

	in_obj = json_object_new_object();
	if(!in_obj)	{
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		render_track();
		return UPNP_E_INTERNAL_ERROR;
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "dinglei1\n");
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if(!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(""));
	}	else {
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);
	value_free(in_arg);

	in_arg = UpnpUtil_GetFirstDocumentItem(in,"Channel");
	if (!in_arg)	{
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "Channel", json_object_new_string(""));
	}	else {
		json_object_object_add(in_obj, "Channel", json_object_new_string(in_arg));

		eventArg = json_object_to_json_string(in_obj);
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Channel = %s\n", eventArg.c_str());
		if (DMREvent_Callback) {
			DMREvent_Callback(DLNA_EVENT_DMR_GETVOLUME,eventArg.c_str(), &result);
		}
	}
	value_free(in_arg);

	if( UpnpAddToActionResponse( out, "GetVolume",
      render_service_type[RENDERINGCONTROL_SERVICE], "Channel", in_arg ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
		value_free(in_arg);
		return UPNP_E_INTERNAL_ERROR;
  }

	result_obj = json_tokener_parse(result.c_str());
	if (result_obj) {
		new_obj = json_object_object_get(result_obj, "Volume");
		if (new_obj) {
			value = (char*)json_object_get_string(new_obj);
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "value =%s  \n",value);
		}
	}	else {	//value = "00";
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");
	}
	//upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Mute", &value);
	if (UpnpAddToActionResponse( out, "GetVolume",
     render_service_type[RENDERINGCONTROL_SERVICE], "CurrentVolume", value ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "GetVolume err\n");
		return UPNP_E_INTERNAL_ERROR;
  }
	//value_free(value);
	json_object_put(result_obj);
	render_track();

  return UPNP_E_SUCCESS;
}

/**********************************************************************
      <action>
         <name>ListPresets</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>                <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>CurrentPresetNameList</name>               <direction>out</direction>
               <relatedStateVariable>PresetNameList</relatedStateVariable>
            </argument>
         </argumentList>
***********************************************************************/
int ListPresets( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();

	if (UpnpAddToActionResponse( out, "ListPresets",
     render_service_type[RENDERINGCONTROL_SERVICE],
     "CurrentPresetNameList", "FactoryDefaults,InstallationDefaults" ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
	}
	return UPNP_E_SUCCESS;
}

/*********************************************************************
         <name>SelectPreset</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>PresetName</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_PresetName</relatedStateVariable>
            </argument>
         </argumentList>
***********************************************************************/
int SelectPreset( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	if( UpnpAddToActionResponse( out, "SelectPreset",
    render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
	}
  return UPNP_E_SUCCESS;
}

/*********************************************************************
         <name>SetBlueVideoBlackLevel</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredBlueVideoBlackLevel</name>               <direction>in</direction>
               <relatedStateVariable>BlueVideoBlackLevel</relatedStateVariable>
            </argument>
         </argumentList>
**********************************************************************/
int SetBlueVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/**********************************************************************
         <name>SetBrightness</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredBrightness</name>               <direction>in</direction>
               <relatedStateVariable>Brightness</relatedStateVariable>
            </argument>
         </argumentList>
***********************************************************************/
int SetBrightness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	char *value = NULL;


	render_track();
	value = UpnpUtil_GetFirstDocumentItem( in, "DesiredBrightness" );
	if(value){
		int b_int = -1;
		b_int = atoi(value);
		if(b_int >= 0){
			//if(yhw_vout_setVideoBrightness(b_int) == 0){
#ifdef NODEFINE_DINGLEI
  int index = -1;
  if(HySDK_Decoder_SetVideoBrightness(b_int) == 0){
  index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Brightness", value);
  if(index < 0){
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }
  }
#endif
		}
		value_free(value);
	}
	#if 0
	UpnpNotify( render_handle, upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn, upnp_service_table[RENDERINGCONTROL_SERVICE].service_id,
	            ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_name[index], ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_value[index], 1 );
	#endif
  if( UpnpAddToActionResponse( out, "SetBrightness",
     render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}

/************************************************************************
         <name>SetColorTemperature</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredColorTemperature</name>               <direction>in</direction>
               <relatedStateVariable>ColorTemperature</relatedStateVariable>
            </argument>
         </argumentList>
*****************************************************************************/
int SetColorTemperature( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/**************************************************************************
         <name>SetContrast</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredContrast</name>               <direction>in</direction>
               <relatedStateVariable>Contrast</relatedStateVariable>
            </argument>
         </argumentList>
****************************************************************************/
int SetContrast( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	char *value = NULL;

	render_track();
	value = UpnpUtil_GetFirstDocumentItem( in, "DesiredContrast" );
	if(value){
		int b_int = -1;
		b_int = atoi(value);
		if(b_int >= 0){
			//if(yhw_vout_setVideoContrast(b_int) == 0){
			#ifdef NODEFINE_DINGLEI
			int index = -1;
			if(HySDK_Decoder_SetVideoContrast(b_int) == 0){
				index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Contrast", value);
				if(index < 0){
          ( *out ) = NULL;
          ( *errorString ) = "Internal Error";
          value_free(value);
          return UPNP_E_INTERNAL_ERROR;
				}
			}
			#endif
		}
		value_free(value);
	}
	#if 0
	UpnpNotify( render_handle, upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn, upnp_service_table[RENDERINGCONTROL_SERVICE].service_id,
	            ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_name[index], ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_value[index], 1 );
	#endif
  if( UpnpAddToActionResponse( out, "SetContrast",
      render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}

/****************************************************************************
         <name>SetGreenVideoBlackLevel</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredGreenVideoBlackLevel</name>               <direction>in</direction>
               <relatedStateVariable>GreenVideoBlackLevel</relatedStateVariable>
            </argument>
         </argumentList>
*******************************************************************************/
int SetGreenVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/**************************************************************************
         <name>SetMute</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>Channel</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredMute</name>               <direction>in</direction>
               <relatedStateVariable>Mute</relatedStateVariable>
            </argument>
         </argumentList>
***************************************************************************/
int SetMute( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *value = NULL;
	char *in_arg;
	std::string result;
	std::string eventArg;
	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
	int index = -1;

	in_obj = json_object_new_object();
	if (!in_obj) {
		value_free(value);
		render_track();
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if (!in_arg)	{
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(""));
	}	else {
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"Channel");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "Channel", json_object_new_string(""));
	}	else {
		json_object_object_add(in_obj, "Channel", json_object_new_string(in_arg));
		index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Mute channel", in_arg);
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Channel = %s\n", in_arg);
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"DesiredMute");
	if (!in_arg)	{
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
    json_object_object_add(in_obj, "DesiredMute", json_object_new_string(""));
	}	else {
    json_object_object_add(in_obj, "Mute", json_object_new_string(in_arg));
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Mute = %s\n", in_arg);
    eventArg = json_object_to_json_string(in_obj);
    //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg);
    if (DMREvent_Callback) {
      DMREvent_Callback(DLNA_EVENT_DMR_SETMUTE,eventArg.c_str(), &result);
    }
    if (0 == strcmp("1", in_arg))
    index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Mute", "true");
    else
    index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Mute", "false");
	}
	value_free(in_arg);
	if (UpnpAddToActionResponse( out, "SetMute",
      render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    json_object_put(result_obj);
    return UPNP_E_INTERNAL_ERROR;
	}
  std::string buf;
  std::string dst_buf;

  rcs_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "LastChange", dst_buf.c_str());
  render_track();

	UpnpNotify( render_handle,
      upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn,
      upnp_service_table[RENDERINGCONTROL_SERVICE].service_id,
      ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_name[index],
      ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_value[index], 1 );


	json_object_put(result_obj);
	render_track();
	return UPNP_E_SUCCESS;
}

/***************************************************************************
         <name>SetRedVideoBlackLevel</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredRedVideoBlackLevel</name>               <direction>in</direction>
               <relatedStateVariable>RedVideoBlackLevel</relatedStateVariable>
            </argument>
         </argumentList>
*****************************************************************************/
int SetRedVideoBlackLevel( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	return 0;
}

/*****************************************************************************
         <name>SetSharpness</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>               <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredSharpness</name>               <direction>in</direction>
               <relatedStateVariable>Sharpness</relatedStateVariable>
            </argument>
         </argumentList>
*******************************************************************************/
int SetSharpness( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	char *value = NULL;


	render_track();
	value = UpnpUtil_GetFirstDocumentItem( in, "DesiredSharpness" );
	if(value){
		int b_int = -1;
		b_int = atoi(value);
		if(b_int >= 0){
			//if(yhw_vout_setVideoBrightness(b_int) == 0){
			#ifdef NODEFINE_DINGLEI
			int index = -1;
			if(HySDK_Decoder_SetVideoSharpness(b_int) == 0){
				index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Sharpness", value);
				if(index < 0){
          ( *out ) = NULL;
          ( *errorString ) = "Internal Error";
          return UPNP_E_INTERNAL_ERROR;
				}
			}
			#endif
		}
		value_free(value);
	}
	#if 0
	UpnpNotify( render_handle, upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn, upnp_service_table[RENDERINGCONTROL_SERVICE].service_id,
	            ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_name[index], ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_value[index], 1 );
	#endif
  if( UpnpAddToActionResponse( out, "SetSharpness",
     render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  return UPNP_E_SUCCESS;
}

/**********************************************************************
         <name>SetVolume</name>
         <argumentList>
            <argument>
               <name>InstanceID</name>  <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>
            </argument>
            <argument>
               <name>Channel</name>  <direction>in</direction>
               <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>
            </argument>
            <argument>
               <name>DesiredVolume</name>               <direction>in</direction>
               <relatedStateVariable>Volume</relatedStateVariable>
            </argument>
         </argumentList>
**************************************************************************/
int SetVolume( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *value = NULL;
	char *in_arg;
	std::string result;
	std::string eventArg;
	int index = -1;
	//struct json_object* result_obj = NULL,
  //struct json_object* new_obj = NULL,
  struct json_object* in_obj = NULL;
	in_obj = json_object_new_object();
	if (!in_obj) {
		value_free(value);
		render_track();
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"Channel");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "Channel", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "Channel", json_object_new_string(in_arg));
		index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Volume channel", in_arg);
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Channel = %s\n", in_arg);
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"DesiredVolume");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "DesiredVolume", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "Volume", json_object_new_string(in_arg));
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Volume = %s\n", in_arg);
		eventArg = json_object_to_json_string(in_obj);
		//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg);
		if (DMREvent_Callback) {
			DMREvent_Callback(DLNA_EVENT_DMR_SETVOLUME,eventArg.c_str(), &result);
		}
		upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Volume", in_arg);
	}
	value_free(in_arg);
  json_object_put( in_obj);
  in_obj = NULL;
	if( UpnpAddToActionResponse( out, "SetVolume",
     render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
	}
#if 1
  std::string buf;
  std::string dst_buf;

  rcs_lastchange_build_cpp(buf, 64 * 1024, "0");
  render_track();
  xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str());
  render_track();
  index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "LastChange", dst_buf.c_str());
  render_track();
#else
#endif
	UpnpNotify( render_handle, upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn,
    upnp_service_table[RENDERINGCONTROL_SERVICE].service_id,
    ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_name[index],
    ( const char ** )&upnp_service_table[RENDERINGCONTROL_SERVICE].var_value[index], 1 );

	render_track();
	return UPNP_E_SUCCESS;
}
int GetAllowedTransforms( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	char *value = NULL;
//	int index = -1;

	render_track();
	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "ZoomMAX", &value);
  if( UpnpAddToActionResponse( out, "GetAllowedTransforms",
     render_service_type[RENDERINGCONTROL_SERVICE], "ZoomMAX", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "ZoomMIN", &value);
	if (UpnpAddToActionResponse( out, "GetAllowedTransforms",
	   render_service_type[RENDERINGCONTROL_SERVICE], "ZoomMIN", value ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "VolumeMAX", &value);
	if (UpnpAddToActionResponse( out, "GetAllowedTransforms",
	   render_service_type[RENDERINGCONTROL_SERVICE], "VolumeMAX", value ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "VolumeMIN", &value);
	if( UpnpAddToActionResponse( out, "GetAllowedTransforms",
     render_service_type[RENDERINGCONTROL_SERVICE], "VolumeMIN", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
	}	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "ClosedCaptioning", &value);
  if( UpnpAddToActionResponse( out, "GetAllowedTransforms",
     render_service_type[RENDERINGCONTROL_SERVICE], "ClosedCaptioning", value ) != UPNP_E_SUCCESS ) {
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    value_free(value);
    return UPNP_E_INTERNAL_ERROR;
  }	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "AudioTrackSelection", &value);
	if (UpnpAddToActionResponse( out, "GetAllowedTransforms",
	    render_service_type[RENDERINGCONTROL_SERVICE], "AudioTrackSelection", value ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Rotation", &value);
	if( UpnpAddToActionResponse( out, "GetAllowedTransforms", render_service_type[RENDERINGCONTROL_SERVICE], "Rotation", value ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "X_Philips.com_AmbiLight", &value);
	if( UpnpAddToActionResponse( out, "GetAllowedTransforms", render_service_type[RENDERINGCONTROL_SERVICE], "X_Philips.com_AmbiLight", value ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}	render_track();

	upnp_var_get(&upnp_service_table[RENDERINGCONTROL_SERVICE], "Brightness", &value);
	if( UpnpAddToActionResponse( out, "GetAllowedTransforms", render_service_type[RENDERINGCONTROL_SERVICE], "Brightness", value ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}

	return UPNP_E_SUCCESS;
}

int SetTransforms( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *value = NULL;
	char *in_arg = NULL;
	std::string result;
	std::string eventArg;
	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
	in_obj = json_object_new_object();
	if (!in_obj) {
		render_track();
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	in_arg = UpnpUtil_GetFirstDocumentItem(in,"InstanceID");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "InstanceID", json_object_new_string(in_arg));
	}
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "InstanceID = %s\n", in_arg);
	value_free(in_arg);

	in_arg = UpnpUtil_GetFirstDocumentItem(in,"Zoom");
	if (!in_arg) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
		json_object_object_add(in_obj, "Zoom", json_object_new_string(""));
	} else {
		json_object_object_add(in_obj, "Zoom", json_object_new_string(in_arg));
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "Zoom = %s\n", in_arg);
		eventArg = json_object_to_json_string(in_obj);
		//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "eventArg = %s\n", eventArg);
		if (DMREvent_Callback) {
			DMREvent_Callback(DLNA_EVENT_DMR_SETTRANSFORMS_ZOOM,eventArg.c_str(), &result);
		}
	}
	value_free(in_arg);
  json_object_put(in_obj);
  in_obj = NULL;
	//TO_DO 其他功能与上面类似
	if( UpnpAddToActionResponse( out, "SetTransforms", render_service_type[RENDERINGCONTROL_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
	    ( *out ) = NULL;
	    ( *errorString ) = "Internal Error";
	    return UPNP_E_INTERNAL_ERROR;
	}
	render_track();

	return UPNP_E_SUCCESS;
}

/***************************************************************************
#define GETPRODUCTINFO \
"<xs:complexType name\"Product\"> \
<xs:element name=\"ProductId\" type=\"xs:string\" value=\"%s\" />	\
<xs:element name=\"ServiceId\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"ContentId\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"ColumnId\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"ProductName\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"ProductDesc\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"Type\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"Price\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"Expires\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"RentalTerm\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"Times\" type=\"xs:string\" value=\"%s\" /> \
<xs:element name=\"IsFree\" type=\"xs:string\" value=\"%s\" /> \
</xs:complexType>"
***************************************************************************/
int X_CTC_GetProductInfo( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
  render_track();
  char *in_arg = NULL;
  struct json_object* result_obj = NULL;
  struct json_object* new_obj = NULL;
  struct json_object* in_obj = NULL;
  std::string result;
  std::string eventArg;
  char *value = NULL;
  int ret = 0;
  int num = 0, i = 0;
  in_obj = json_object_new_object();
  if (!in_obj) {
    value_free(value);
    render_track();
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }
  in_arg = UpnpUtil_GetFirstDocumentItem(in,"CurrentURI");
  if (!in_arg) {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "in_arg NULL \n");
    json_object_object_add(in_obj, "CurrentURI", json_object_new_string(""));
  } else {
    json_object_object_add(in_obj, "CurrentURI", json_object_new_string(in_arg));
  }
  eventArg = json_object_to_json_string(in_obj);
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentURI = %s\n", in_arg);
  value_free(in_arg);
  if (DMREvent_Callback) {
    DMREvent_Callback(DLNA_EVENT_DMR_GETPRODUCTINFO,eventArg.c_str(), &result);
  }
  json_object_put(in_obj);
  in_obj = NULL;
  result_obj = json_tokener_parse(result.c_str());
  if (result_obj) {
    new_obj = json_object_object_get(result_obj, "returncode");
    if (new_obj) {
      value = (char*)json_object_get_string(new_obj);
    }
  }	else {
    render_track();
    json_object_put(result_obj);
    result_obj = NULL;
    ( *out ) = NULL;
    ( *errorString ) = "Internal Error";
    return UPNP_E_INTERNAL_ERROR;
  }

	json_object_put(result_obj);
	result_obj = NULL;
	return 0;
}
int X_CTC_Order( IN IXML_Document * in, OUT IXML_Document ** out, OUT char **errorString )
{
	render_track();
	char *in_arg = NULL;
	struct json_object *result_obj = NULL,*new_obj = NULL,*in_obj = NULL;
	std::string result;
	std::string eventArg;
	int ret = 0;
	char *value = NULL;

	in_obj = json_object_new_object();
	if (!in_obj) {
//		value_free(value);
		render_track();
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		return UPNP_E_INTERNAL_ERROR;
	}

	in_arg = UpnpUtil_GetFirstDocumentItem(in,"Action");
	if (in_arg) {
		json_object_object_add(in_obj, "Action", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "Action", json_object_new_string(""));
	}
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"ProductId");
	if (in_arg) {
		json_object_object_add(in_obj, "ProductId", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "ProductId", json_object_new_string(""));
	}
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"ServiceId");
	if(in_arg) {
		json_object_object_add(in_obj, "ServiceId", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "ServiceId", json_object_new_string(""));
	}
	value_free(in_arg);
  in_arg = UpnpUtil_GetFirstDocumentItem(in,"ContentId");
	if (in_arg) {
		json_object_object_add(in_obj, "ContentId", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "ContentId", json_object_new_string(""));
	}
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"ColumnId");
	if (in_arg) {
		json_object_object_add(in_obj, "ColumnId", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "ColumnId", json_object_new_string(""));
	}
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"PurchaseType");
	if (in_arg) {
		json_object_object_add(in_obj, "PurchaseType", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "PurchaseType", json_object_new_string(""));
	}
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"ContentType");
	if (in_arg) {
		json_object_object_add(in_obj, "ContentType", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "ContentType", json_object_new_string(""));
	}
	value_free(in_arg);
	in_arg = UpnpUtil_GetFirstDocumentItem(in,"AutoRenewal");
	if (in_arg) {
    json_object_object_add(in_obj, "AutoRenewal", json_object_new_string(in_arg));
	} else {
		json_object_object_add(in_obj, "AutoRenewal", json_object_new_string(""));
	}
	value_free(in_arg);
	eventArg = json_object_to_json_string(in_obj);
	//HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "CurrentURI = %s\n", in_arg);
	//return UPNP_E_INTERNAL_ERROR;
	if (DMREvent_Callback) {
    DMREvent_Callback(DLNA_EVENT_DMR_ORDER, eventArg.c_str(), &result);
	}
  json_object_put(in_obj);
  in_obj = NULL;
	result_obj = json_tokener_parse(result.c_str());
	if (result_obj) {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "  \n");
		new_obj = json_object_object_get(result_obj, "returncode");
		if (new_obj) {
			value = (char*)json_object_get_string(new_obj);
			HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "returncode =%s  \n",value);
		}
	} else {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "return ERR \n");
		return UPNP_E_INTERNAL_ERROR;
	}
	ret = atoi(value);
	if (UpnpAddToActionResponse( out, "Order",
     render_service_type[X_CTC_SUBSCRIBE_SERVICE], NULL, NULL ) != UPNP_E_SUCCESS ) {
		( *out ) = NULL;
		( *errorString ) = "Internal Error";
		//value_free(value);
		render_track();
		json_object_put(result_obj);
		return UPNP_E_INTERNAL_ERROR;
  }
	render_track();
	json_object_put(result_obj);
	return ret;
}

/******************************************************************************
 * RenderHandleSubscriptionRequest
 *
 * Description:
 *       Called during a subscription request callback.  If the
 *       subscription request is for this device and either its
 *       control service or picture service, then accept it.
 *
 * Parameters:
 *   sr_event -- The subscription request event structure
 *
 *****************************************************************************/
int RenderHandleSubscriptionRequest( IN struct Upnp_Subscription_Request *sr_event )
{
  unsigned int i = 0;

  // IXML_Document *PropSet=NULL;
  render_track();
  //lock state mutex
  ithread_mutex_lock( &render_mux );

  for( i = 0; i < RENDER_SERVICE_CNT; i++ ) {
    if( ( strcmp( sr_event->UDN, upnp_service_table[i].service_udn) == 0 ) &&
      ( strcmp( sr_event->ServiceId, upnp_service_table[i].service_id) == 0 ) ) {
      if(i == AVTRANSPORT_SERVICE){
        std::string buf;
        std::string dst_buf;
        int index;
        render_track();

        if (av_lastchange_build_cpp(buf, 64 * 1024, "0") != 0){
          render_msg("error to build lastchange\n");
          ithread_mutex_unlock( &render_mux );
          return (-1);
        }
        if(xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str()) == 0){
          index = upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "LastChange", dst_buf.c_str());
          UpnpAcceptSubscription( render_handle, sr_event->UDN, sr_event->ServiceId,
            ( const char ** )&(upnp_service_table[AVTRANSPORT_SERVICE].var_name[index]),\
            ( const char ** )&(upnp_service_table[AVTRANSPORT_SERVICE].var_value[index]),
            1 , sr_event->Sid );
        }

      } else if (i == RENDERINGCONTROL_SERVICE){
        std::string buf;
        std::string dst_buf;
        int index;
        render_track();

        if (rcs_lastchange_build_cpp(buf, 64 * 1024, "0") != 0){
          render_msg("error to build lastchange\n");
          ithread_mutex_unlock( &render_mux );
          return (-1);
        }
        if(xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str()) == 0){
          index = upnp_var_set(&upnp_service_table[RENDERINGCONTROL_SERVICE], "LastChange", dst_buf.c_str());
          UpnpAcceptSubscription( render_handle, sr_event->UDN, sr_event->ServiceId,
            ( const char ** )&(upnp_service_table[RENDERINGCONTROL_SERVICE].var_name[index]),\
            ( const char ** )&(upnp_service_table[RENDERINGCONTROL_SERVICE].var_value[index]),
            1 , sr_event->Sid );
        }
      } else if (i == X_CTC_SUBSCRIBE_SERVICE){
        std::string buf;
        std::string dst_buf;
        int index;
        render_track();

        if(x_ctc_ss_lastchange_build_cpp(buf, 64 * 1024, "0") != 0){
          render_msg("error to build lastchange\n");
          ithread_mutex_unlock( &render_mux );
          return (-1);
        }
        if(xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str()) == 0){
          index = upnp_var_set(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], "LastChange", dst_buf.c_str());
          UpnpAcceptSubscription( render_handle, sr_event->UDN, sr_event->ServiceId, ( const char ** )&(upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].var_name[index]),\
            ( const char ** )&(upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].var_value[index]), 1 , sr_event->Sid );
        }
      } else if (i == CONNECTIONMANAGER_SERVICE){
        render_track();

        std::string buf;
        std::string dst_buf;
        int index;
        render_track();

        if(cms_lastchange_build_cpp(buf, 64 * 1024, "0") != 0){
          render_msg("error to build lastchange\n");
          ithread_mutex_unlock( &render_mux );
          return (-1);
        }
        if(xml_change_copy_cpp(dst_buf, 72 * 1024, buf.c_str()) == 0){
          index = upnp_var_set(&upnp_service_table[CONNECTIONMANAGER_SERVICE], "LastChange", dst_buf.c_str());
          UpnpAcceptSubscription( render_handle, sr_event->UDN, sr_event->ServiceId,
            ( const char ** )&(upnp_service_table[CONNECTIONMANAGER_SERVICE].var_name[index]),\
            ( const char ** )&(upnp_service_table[CONNECTIONMANAGER_SERVICE].var_value[index]),
            1 , sr_event->Sid );
        }
      }
    }
  }

  ithread_mutex_unlock( &render_mux );

  return ( 1 );
}

/******************************************************************************
 * RenderHandleGetVarRequest
 *
 * Description:
 *       Called during a get variable request callback.  If the
 *       request is for this device and either its control service
 *       or picture service, then respond with the variable value.
 *
 * Parameters:
 *   cgv_event -- The control get variable request event structure
 *
 *****************************************************************************/
int RenderHandleGetVarRequest( INOUT struct Upnp_State_Var_Request *cgv_event )
{
  int i = 0, j = 0;
  int getvar_succeeded = 0;

  cgv_event->CurrentVal = NULL;
  render_track();

  ithread_mutex_lock( &render_mux );

  for( i = 0; i < RENDER_SERVICE_CNT; i++ ) {
    //check udn and service id
    if( ( strcmp( cgv_event->DevUDN, upnp_service_table[i].service_udn) == 0 )
      && ( strcmp( cgv_event->ServiceID, upnp_service_table[i].service_id) == 0 ) ) {
      //check variable name
      for( j = 0; j < upnp_service_table[i].var_cnt; j++ ) {
        if( strcmp( cgv_event->StateVarName, upnp_service_table[i].var_name[j] ) == 0 ) {
          getvar_succeeded = 1;
          cgv_event->CurrentVal = ixmlCloneDOMString( upnp_service_table[i].var_value[j] );
          break;
        }
      }
    }
  }
#if defined(DEBUG)
  struct sockaddr_in addr;
  char val[256];
  memcpy( &addr, &cgv_event->CtrlPtIPAddr, sizeof(struct sockaddr_in));
  int ip = ntohl(addr.sin_addr.s_addr);
  snprintf (val, sizeof(val), "%d.%d.%d.%d", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_KEY, "action[%d], cgv_event->StateVarName[%s], ip[%s]\n", i, cgv_event->StateVarName, val);
#endif

  if( getvar_succeeded ) {
    cgv_event->ErrCode = UPNP_E_SUCCESS;
  } else {
    UpnpUtil_Print( "Error in UPNP_CONTROL_GET_VAR_REQUEST callback:\n" );
    UpnpUtil_Print( "   Unknown variable name = %s\n", cgv_event->StateVarName );
    cgv_event->ErrCode = 404;
    strcpy( cgv_event->ErrStr, "Invalid Variable" );
  }

  ithread_mutex_unlock( &render_mux );

  return ( cgv_event->ErrCode == UPNP_E_SUCCESS );
}

/******************************************************************************
 * RenderHandleActionRequest
 *
 * Description:
 *       Called during an action request callback.  If the
 *       request is for this device and either its control service
 *       or picture service, then perform the action and respond.
 *
 * Parameters:
 *   ca_event -- The control action request event structure
 *
 *****************************************************************************/
int RenderHandleActionRequest( INOUT struct Upnp_Action_Request *ca_event )
{
  /*
     Defaults if action not found
     */
  int action_found = 0;
  int i = 0;
  int service = -1;
  int retCode = 0;
  char *errorString = NULL;

  render_track();

  ca_event->ErrCode = 0;
  ca_event->ActionResult = NULL;

  if( ( strcmp( ca_event->DevUDN,  upnp_service_table[AVTRANSPORT_SERVICE].service_udn) == 0 ) &&
    ( strcmp( ca_event->ServiceID, upnp_service_table[AVTRANSPORT_SERVICE].service_id) == 0 ) ) {
    service = AVTRANSPORT_SERVICE;
  } else if( ( strcmp( ca_event->DevUDN, upnp_service_table[CONNECTIONMANAGER_SERVICE].service_udn) == 0 ) &&
    ( strcmp( ca_event->ServiceID, upnp_service_table[CONNECTIONMANAGER_SERVICE].service_id) == 0 ) ) {
    service = CONNECTIONMANAGER_SERVICE;
  }  else if( ( strcmp( ca_event->DevUDN, upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn) == 0 ) &&
    ( strcmp( ca_event->ServiceID, upnp_service_table[RENDERINGCONTROL_SERVICE].service_id) == 0 ) ) {
    service = RENDERINGCONTROL_SERVICE;
  } else  if( ( strcmp( ca_event->DevUDN, upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].service_udn) == 0 ) &&
    ( strcmp( ca_event->ServiceID, upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].service_id) == 0 ) ) {
    service = X_CTC_SUBSCRIBE_SERVICE;
  } else {
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "error event:udn is %s, service id is %s\n",  ca_event->DevUDN,  ca_event->ServiceID);
    for(i = 0; i < RENDER_SERVICE_CNT; i ++){
      HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%d udn is %s, service id is %s\n", i, upnp_service_table[i].service_udn, upnp_service_table[i].service_id);
    }
  }
  //Find and call appropriate procedure based on action name
  //Each action name has an associated procedure stored in the
  //service table. These are set at initialization.
#if defined(DEBUG)
  struct sockaddr_in in;
  char val[256];
  memcpy( &in, &ca_event->CtrlPtIPAddr, sizeof(struct sockaddr_in));
  int ip = ntohl (in.sin_addr.s_addr);
  snprintf (val, sizeof(val), "%d.%d.%d.%d", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_KEY, "action[%d], ca_event->ActionName[%s], ip[%s]\n", service, ca_event->ActionName, val);
#endif
  for( i = 0; ((i < upnp_service_table[service].action_cnt)); i++ ) {

    if (upnp_service_table[service].action[i].action_name
      && !strcmp( ca_event->ActionName, upnp_service_table[service].action[i].action_name) ) {
      if( upnp_service_table[service].action[i].callback)
        retCode = upnp_service_table[service].action[i].callback( ca_event->ActionRequest, &ca_event->ActionResult, &errorString );
      action_found = 1;
      break;
    }
  }
  render_track();
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "retCode %d\n",retCode);

  if( !action_found ) {
    ca_event->ActionResult = NULL;
    strcpy( ca_event->ErrStr, "Invalid Action" );
    ca_event->ErrCode = 401;
  } else {
    if( retCode == UPNP_E_SUCCESS ) {
      ca_event->ErrCode = UPNP_E_SUCCESS;
    } else {
      //copy the error string
      if(errorString)
        strcpy( ca_event->ErrStr, errorString );
      switch ( retCode ) {
      case UPNP_E_INVALID_PARAM:
        {
          ca_event->ErrCode = 402;
          break;
        }
      case UPNP_E_INTERNAL_ERROR:
        ca_event->ErrCode = 501;
        break;
      default:
        {
          HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "retCode %d\n",retCode);

          ca_event->ErrCode = retCode;
          HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "ca_event->ErrCode %d\n",ca_event->ErrCode);

          break;
        }
      }
    }
  }
  render_track();

  return ( ca_event->ErrCode );
}

/*--------------------------------------*/
static void AVTransport_var_init(void)
{
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 0, "A_ARG_TYPE_InstanceID", "0", 1);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 1, "A_ARG_TYPE_SeekMode", "0", 0);//ABS_TIME,REL_TIME,ABS_COUNT,REL_COUNT,TRACK_NR,CHANNEL_FREQ,TAPE-INDEX,FRAME
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 2, "A_ARG_TYPE_SeekTarget", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 3, "AbsoluteCounterPosition", "2147483647", 2);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 4, "AbsoluteTimePosition", "NOT_IMPLEMENTED", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 5, "AVTransportURI", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 6, "AVTransportURIMetaData", 0, 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 7, "CurrentMediaDuration", "00:00:00", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 8, "CurrentPlayMode", "NORMAL", 0);// NORMAL REPEAT_ALL INTRO
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 9, "CurrentRecordQualityMode", "0:BASIC", 0);//0:EP,1:LP,2:SP,0:BASIC,1:MEDIUM,2:HIGH,NOT_IMPLEMENTED, vendor-defined
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 10, "CurrentTrack", "0", 1);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 11, "CurrentTrackDuration", "00:00:00", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 12, "CurrentTrackMetaData", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 13, "CurrentTrackURI", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 14, "CurrentTransportActions", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 15, "LastChange", 0, 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 16, "NextAVTransportURI", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 17, "NextAVTransportURIMetaData", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 18, "NumberOfTracks", "0", 1);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 19, "PlaybackStorageMedium", "UNKNOWN", 0);//UNKNOWN,DV,MINI-DV,VHS,W-VHS,S-VHS,D-VHS,VHSC,VIDEO8,HI8,CD-ROM,CD-DA,CD-R,CD-RW,
		//VIDEO-CD,SACD,MD-AUDIO,MD-PICTURE,DVD-ROM, DVD-VIDEO,DVD-R,DVD+RW,DVD-RW,DVD-RAM,DVD-AUDIO,DAT,LD,HDD,MICRO-MV,NETWORK,NONE,NOT_IMPLEMENTED,vendor-defined
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 20, "PossiblePlaybackStorageMedia", "UNKNOWN,DV,MINI-DV,VHS,W-VHS,S-VHS,D-VHS,VHSC,\
		VIDEO8,HI8,CD-ROM,CD-DA,CD-R,CD-RW, VIDEO-CD,SACD,MD-AUDIO,MD-PICTURE,DVD-ROM, DVD-VIDEO,DVD-R,DVD+RW,DVD-RW,DVD-RAM,DVD-AUDIO,DAT,LD,HDD,MICRO-MV,NETWORK,NONE", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 21, "PossibleRecordQualityModes", "NOT_IMPLEMENTED", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 22, "PossibleRecordStorageMedia", "NOT_IMPLEMENTED", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 23, "RecordMediumWriteStatus", "UNKNOWN", 0);	//WRITABLE,PROTECTED,NOT_WRITABLE,UNKNOWN,NOT_IMPLEMENTED
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 24, "RecordStorageMedium", "UNKNOWN", 0);// UNKNOWN, DV,MINI-DV,VHS,W-VHS,S-VHS,D-VHS,VHSC,VIDEO8, HI8, CD-ROM,CD-DA, CD-R, CD-RW,  VIDEO-CD,
	// SACD,  MD-AUDIO, MD-PICTURE, DVD-ROM,  DVD-VIDEO,DVD-R, DVD+RW,  DVD-RW, DVD-RAM, DVD-AUDIO,DAT,  LD,  HDD, MICRO-MV, NETWORK, NONE, NOT_IMPLEMENTED, vendor-defined
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 25, "RelativeCounterPosition", "2147483647", 2);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 26, "RelativeTimePosition", "NOT_IMPLEMENTED", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 27, "TransportPlaySpeed", "1", 0);/*1,   vendor-defined */
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 28, "TransportState", "NO_MEDIA_PRESENT", 0);//STOPPED,PAUSED_PLAYBACK,PAUSED_RECORDING,PLAYING,RECORDING,TRANSITIONING,NO_MEDIA_PRESENT
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 29, "TransportStatus", "OK", 0);//OK,ERROR_OCCURRED,vendor-defined
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 30, "X_DLNA_CurrentTrackSize", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 31, "X_DLNA_RelativeBytePosition", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 32, "X_DLNA_AbsoluteBytePosition", "0", 0);
	upnp_var_init(&upnp_service_table[AVTRANSPORT_SERVICE], 33, "InstanceID", "0", 0);
	upnp_service_table[AVTRANSPORT_SERVICE].var_cnt = 34;
}


static void AVTransport_action_init(void)
{
	memset(AVTransport_action, 0, sizeof(struct upnp_action) * AVTRANSPORT_ACTION_CNT);
	upnp_action_set(&AVTransport_action[0], "GetCurrentTransportActions", GetCurrentTransportActions);
	upnp_action_set(&AVTransport_action[1], "GetDeviceCapabilities", GetDeviceCapabilities);
	upnp_action_set(&AVTransport_action[2], "GetMediaInfo", GetMediaInfo);
	upnp_action_set(&AVTransport_action[3], "GetPositionInfo", GetPositionInfo);
	upnp_action_set(&AVTransport_action[4], "GetTransportInfo", GetTransportInfo);
	upnp_action_set(&AVTransport_action[5], "GetTransportSettings", GetTransportSettings);
	upnp_action_set(&AVTransport_action[6], "Next", Player_Next);
	upnp_action_set(&AVTransport_action[7], "Pause", Player_Pause);
	upnp_action_set(&AVTransport_action[8], "Play", Player_Play);
	upnp_action_set(&AVTransport_action[9], "Previous", Player_Previous);
	upnp_action_set(&AVTransport_action[10], "Seek", Player_Seek);
	upnp_action_set(&AVTransport_action[11], "SetAVTransportURI", SetAVTransportURI);
	upnp_action_set(&AVTransport_action[12], "SetPlayMode", SetPlayMode);
	upnp_action_set(&AVTransport_action[13], "Stop", Player_Stop);
	upnp_action_set(&AVTransport_action[14], "X_DLNA_GetBytePositionInfo", X_DLNA_GetBytePositionInfo);
	upnp_action_set(&AVTransport_action[15], "SetTransportURI", SetAVTransportURI);

	upnp_service_table[AVTRANSPORT_SERVICE].action = AVTransport_action;
	upnp_service_table[AVTRANSPORT_SERVICE].action_cnt = AVTRANSPORT_ACTION_CNT;
}

//#define LPCM_ENABLE

#if LPCM_ENABLE
static const char *SinkProtocolInfo = "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM,http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM,\
http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM,http-get:*:audio/L16;rate=48000;channels=1:DLNA.ORG_PN=LPCM,http-get:*:audio/L16;\
rate=48000;channels=2:DLNA.ORG_PN=LPCM,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_520,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_KO_ISO,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO,\
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN,\
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM_ICO,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG_ICO,http-get:*:image/png:DLNA.ORG_PN=PNG_TN,\
http-get:*:image/png:DLNA.ORG_PN=PNG_SM_ICO,http-get:*:image/png:DLNA.ORG_PN=PNG_LRG_ICO,http-get:*:image/png:DLNA.ORG_PN=PNG_LRG,\
http-get:*:image/gif:DLNA.ORG_PN=GIF_LRG,http-get:*:audio/vnd.dolby.dd-raw:DLNA.ORG_PN=AC3,http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_3GPP,\
http-get:*:audio/mp4:DLNA.ORG_PN=AMR_3GPP,http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_WBplus,http-get:*:audio/x-sony-oma:DLNA.ORG_PN=ATRAC3plus,\
http-get:*:audio/L16;rate=8000;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=8000;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=11025;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=11025;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=1200;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=12000;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=16000;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=16000;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=22050;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=22050;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=24000;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=24000;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=32000;channels=1:DLNA.ORG_PN=LPCM_low,http-get:*:audio/L16;rate=32000;channels=2:DLNA.ORG_PN=LPCM_low,\
http-get:*:audio/L16;rate=48000;channels=1:DLNA.ORG_PN=LPCM_MPS,http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM_MPS,\
http-get:*:audio/L16;rate=48000;channels=3:DLNA.ORG_PN=LPCM_MPS,http-get:*:audio/L16;rate=48000;channels=4:DLNA.ORG_PN=LPCM_MPS,\
http-get:*:audio/L16;rate=48000;channels=5:DLNA.ORG_PN=LPCM_MPS,http-get:*:audio/L16;rate=48000;channels=5.1:DLNA.ORG_PN=LPCM_MPS,\
http-get:*:audio/L16;rate=48000;channels=7.1:DLNA.ORG_PN=LPCM_MPS,http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_LTP_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_MULT5_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_MULT5_ISO,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_LTP_MULT5_ISO,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_MULT7_ISO,\
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_MULT7_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_LTP_MULT7_ISO,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_MULT5_ADTS,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MULT5_ISO,\
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_MULT5_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L2_ADTS,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L3_ADTS,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L3_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L3_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_MULT5_ADTS,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MULT5_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MULT5_ISO,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L2_ADTS_320,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO_320,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO_320,http-get:*:audio/mp4:DLNA.ORG_PN=BSAC_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=BSAC_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=BSAC_MULT5_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=BSAC_MULT5_ISO,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MULT7,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MULT7,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_MULT7,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L4,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L4,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L4,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_MULT7,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_MULT7,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_MULT7,http-get:*:audio/mp4:DLNA.ORG_PN=MPEG2_AAC_MPS,\
http-get:*:audio/3gpp:DLNA.ORG_PN=MPEG2_AAC_MPS,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=MPEG2_AAC_MPS,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MPS,\
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_MPS,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_MPS,http-get:*:audio/mp4:DLNA.ORG_PN=ALS_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=ALS_MULT5_ISO,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L4,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L4,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L4,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MPS,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MPS,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_MPS,http-get:*:audio/x-dab:DLNA.ORG_PN=HEAACv2_L2_MPS_DAB,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L2,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2_320,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2_320,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L2_320,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L3,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L3,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L3,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_MULT5,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_MULT5,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_MULT5,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_192,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_192,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_192,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2_128,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2_128,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L2_128,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO_128,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO_128,http-get:*:audio/vnd.dts:DLNA.ORG_PN=DTS,\
http-get:*:audio/vnd.dts-hd-hra:DLNA.ORG_PN=DTSHD_HRA,http-get:*:audio/vnd.dts-hd-lbr:DLNA.ORG_PN=DTSHD_LBR,\
http-get:*:audio/vnd.dts-hd-ma:DLNA.ORG_PN=DTSHD_MA,http-get:*:audio/eac3:DLNA.ORG_PN=EAC3,http-get:*:audio/vnd.dolby.mlp:DLNA.ORG_PN=MLP,\
http-get:*:audio/mpeg:DLNA.ORG_PN=MP2_MPS,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL,\
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_KO_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC_XAC3,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL_XAC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_PAL,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_NTSC,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_PAL_XAC3,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_NTSC_XAC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_X_60_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_X_60_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_X_50_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_X_50_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_JP_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_L2_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_AC3_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_L2_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_AC3_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_60_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_60_L2_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_50_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_50_L2_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_SD_DTS,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTS,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTSHD_HRA,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTSHD_MA,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTSHD,http-get:*:video/x-mpeg2-directv:DLNA.ORG_PN=DIRECTV_TS_SD,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_MPEG1_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_MPEG1_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_MPEG1_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_MPEG1_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_JP_MPEG1_L2_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_DTS_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_DTS_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_DTSHD_HRA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_DTSHD_HRA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_DTSHD_MA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_DTSHD_MA_ISO,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_ATRAC3plus,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC_LTP,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AMR,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AAC,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AAC_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_AAC_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2_ISO,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_ATRAC3plus,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AAC,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AAC_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AAC_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3_ISO,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L5_SO_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L5_SO_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L5_SO_HEAAC_MULT5,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L4_SO_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L4_SO_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L4_SO_HEAAC_MULT5,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L10_AAC,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L10_AAC_LTP,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L10_AMR_WBplus,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_CO_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_MPEG2_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_MPEG2_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_CO_MPEG2_L2_ISO,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P3_L10_AMR,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_H263_3GPP_P3_L10_AMR,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P0_L10_AMR,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P0_L45_AMR,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P0_L45_AMR_WBplus,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L5_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L45_HEAACv2_L2,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR_WBplus,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L0B_HEAACv2_L2,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L3_AMR_WBplus,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L3_HEAACv2_L2,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_HEAAC_res,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC_res,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L6_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L6_AAC_LTP,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L6_HEAAC_L2,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_NDSD,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_HEAAC_L2,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_HEAAC_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_HEAAC_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT5,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT5_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT5_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT7,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT7_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT7_ISO,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L2,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_MPEG1_L3,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AC3,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LTP,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LTP_MULT5,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LTP_MULT7,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_ATRAC3plus,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3_SD_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_BSAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L2_CIF30_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_MULT5,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAAC_L2,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_MPEG1_L3,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AC3,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_LTP,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_LTP_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_BSAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_BSAC_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_940,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AMR,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_LTP,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_LTP_520,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_BSAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L1B_QCIF15_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAACv2_L2,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAACv2_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_HEAAC_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_HEAAC_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_HEAAC_L2_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_MULT5,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_MULT5_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_MULT5_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_HEAAC_L2,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_HEAAC_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_HEAAC_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT5,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT5_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT5_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT7,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT7_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT7_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_LTP,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_LTP_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_LTP_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_BSAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_BSAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_BSAC_ISO,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AAC,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AAC_LTP,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_HEAAC,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF15_AMR_WBplus,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF30_AMR_WBplus,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AMR,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AMR_WBplus,http-get:*:video/mp4:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AMR,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_BSAC_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_BSAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_BSAC,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_L1B_QCIF15_AMR,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_L1B_QCIF15_AMR_WBplus,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_HEAACv2_L4_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_HEAACv2_L4_ISO,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_SD_HEAACv2_L4,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_HEAACv2_L4_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_HEAACv2_L4_ISO,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_HEAACv2_L4,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF15_AMR_WBplus_res,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF30_AMR_WBplus_res,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTS,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTSHD_HRA,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTSHD_MA,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTSHD,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_EAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_EAC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_EAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_EAC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_MPEG1_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_MPEG1_L2_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_SD_EU,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_SD_EU_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_SD_EU_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_EAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_EAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_EAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_EAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_MPEG1_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_MPEG1_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_EU,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_EU_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_EU_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_JP_AAC_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_60_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_50_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_24_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_DTS_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_DTS_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_DTSHD_HRA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_DTSHD_HRA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_DTSHD_MA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_DTSHD_MA_ISO,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L1B_QCIF15_HEAACv2,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAACv2,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAAC_MPS,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MPS,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_MPS,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L4,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_AAC_MPS,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_HEAAC_MPS,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_720p_AAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_1080i_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_AAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_AAC_LTP,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_HEAAC_L2,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_HEAAC_MULT7,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_NDHD,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_L12_CIF15_AMR_WBplus,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_X_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_X_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3_X_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_LPCM_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_LPCM_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_LPCM_T,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAACv2_350,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_350,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAAC_350,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L31_HD_AAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L32_HD_AAC,\
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL_MULT5,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHM_BASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTS,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTSHD_HRA,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTSHD_MA,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTSHD,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L1_AC3_ISO,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L1_WMA_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L2_AC3_ISO,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L2_WMA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=VC1_TS_HD_DTS_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_HD_DTS_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=VC1_TS_HD_DTSHD_HRA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_HD_DTSHD_HRA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=VC1_TS_HD_DTSHD_MA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_HD_DTSHD_MA_ISO,http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA,\
http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L2_WMA";
#else
#if 0
static const char *SinkProtocolInfo = "http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_520,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_KO_ISO,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_ISO,\
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_MED,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_TN,\
http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_SM_ICO,http-get:*:image/jpeg:DLNA.ORG_PN=JPEG_LRG_ICO,http-get:*:image/png:DLNA.ORG_PN=PNG_TN,\
http-get:*:image/png:DLNA.ORG_PN=PNG_SM_ICO,http-get:*:image/png:DLNA.ORG_PN=PNG_LRG_ICO,http-get:*:image/png:DLNA.ORG_PN=PNG_LRG,\
http-get:*:image/gif:DLNA.ORG_PN=GIF_LRG,http-get:*:audio/vnd.dolby.dd-raw:DLNA.ORG_PN=AC3,http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_3GPP,\
http-get:*:audio/mp4:DLNA.ORG_PN=AMR_3GPP,http-get:*:audio/3gpp:DLNA.ORG_PN=AMR_WBplus,http-get:*:audio/x-sony-oma:DLNA.ORG_PN=ATRAC3plus,\
http-get:*:audio/mpeg:DLNA.ORG_PN=MP3X,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_LTP_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_MULT5_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_MULT5_ISO,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_LTP_MULT5_ISO,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_LTP_MULT7_ISO,\
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_LTP_MULT7_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_LTP_MULT7_ISO,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_MULT5_ADTS,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MULT5_ISO,\
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_MULT5_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L2_ADTS,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L3_ADTS,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L3_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L3_ISO,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_MULT5_ADTS,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MULT5_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MULT5_ISO,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L2_ADTS_320,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO_320,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO_320,http-get:*:audio/mp4:DLNA.ORG_PN=BSAC_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=BSAC_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=BSAC_MULT5_ISO,http-get:*:audio/3gpp:DLNA.ORG_PN=BSAC_MULT5_ISO,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MULT7,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MULT7,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_MULT7,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L4,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L4,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L4,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_MULT7,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_MULT7,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_MULT7,http-get:*:audio/mp4:DLNA.ORG_PN=MPEG2_AAC_MPS,\
http-get:*:audio/3gpp:DLNA.ORG_PN=MPEG2_AAC_MPS,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=MPEG2_AAC_MPS,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_MPS,\
http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_MPS,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_MPS,http-get:*:audio/mp4:DLNA.ORG_PN=ALS_ISO,\
http-get:*:audio/mp4:DLNA.ORG_PN=ALS_MULT5_ISO,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L4,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L4,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_L4,http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_MPS,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_MPS,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAAC_MPS,http-get:*:audio/x-dab:DLNA.ORG_PN=HEAACv2_L2_MPS_DAB,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L2,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2_320,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2_320,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L2_320,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L3,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L3,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L3,http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_MULT5,\
http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_MULT5,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_MULT5,\
http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_192,http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO_192,http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO_192,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAACv2_L2_128,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAACv2_L2_128,http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=HEAACv2_L2_128,\
http-get:*:audio/mp4:DLNA.ORG_PN=HEAAC_L2_ISO_128,http-get:*:audio/3gpp:DLNA.ORG_PN=HEAAC_L2_ISO_128,http-get:*:audio/vnd.dts:DLNA.ORG_PN=DTS,\
http-get:*:audio/vnd.dts-hd-hra:DLNA.ORG_PN=DTSHD_HRA,http-get:*:audio/vnd.dts-hd-lbr:DLNA.ORG_PN=DTSHD_LBR,\
http-get:*:audio/vnd.dts-hd-ma:DLNA.ORG_PN=DTSHD_MA,http-get:*:audio/eac3:DLNA.ORG_PN=EAC3,http-get:*:audio/vnd.dolby.mlp:DLNA.ORG_PN=MLP,\
http-get:*:audio/mpeg:DLNA.ORG_PN=MP2_MPS,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL,\
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAPRO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG1,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_KO_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_NTSC_XAC3,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_PAL_XAC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_KO_XAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_XAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_KO_XAC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_XAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_MP_LL_AAC_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_PAL,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_NTSC,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_PAL_XAC3,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_ES_NTSC_XAC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_X_60_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_X_60_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_X_50_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_X_50_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_JP_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_L2_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_60_AC3_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_L2_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_50_AC3_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_60_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_60_L2_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_50_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_50_L2_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_SD_DTS,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTS,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTSHD_HRA,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTSHD_MA,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_PS_HD_DTSHD,http-get:*:video/x-mpeg2-directv:DLNA.ORG_PN=DIRECTV_TS_SD,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_NA_MPEG1_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_NA_MPEG1_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_HD_NA_MPEG1_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_HD_NA_MPEG1_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_EU_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_SD_EU_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_SD_JP_MPEG1_L2_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_DTS_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_DTS_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_DTSHD_HRA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_DTSHD_HRA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG_TS_DTSHD_MA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG_TS_DTSHD_MA_ISO,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_ATRAC3plus,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_AAC_LTP,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L2_AMR,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AAC,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AAC_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_AAC_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_SP_MPEG2_L2_ISO,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_SP_G726,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_HEAAC_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_ATRAC3plus,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AAC,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AAC_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AAC_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_ASP_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_ASP_AC3_ISO,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L5_SO_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L5_SO_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L5_SO_HEAAC_MULT5,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L5_SO_G726,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L4_SO_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L4_SO_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_ASP_L4_SO_HEAAC_MULT5,http-get:*:video/x-ms-asf:DLNA.ORG_PN=MPEG4_P2_ASF_ASP_L4_SO_G726,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L10_AAC,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L10_AAC_LTP,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L10_AMR_WBplus,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_CO_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_MPEG2_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=MPEG4_P2_TS_CO_MPEG2_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=MPEG4_P2_TS_CO_MPEG2_L2_ISO,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AAC,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P3_L10_AMR,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_H263_3GPP_P3_L10_AMR,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P0_L10_AMR,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P0_L45_AMR,http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_H263_3GPP_P0_L45_AMR_WBplus,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L5_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_H263_MP4_P0_L45_HEAACv2_L2,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L0B_AMR_WBplus,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L0B_HEAACv2_L2,\
http-get:*:video/3gpp:DLNA.ORG_PN=MPEG4_P2_3GPP_SP_L3_AMR_WBplus,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L3_HEAACv2_L2,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_HEAAC_res,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_VGA_AAC_res,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L6_AAC,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L6_AAC_LTP,\
http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_SP_L6_HEAAC_L2,http-get:*:video/mp4:DLNA.ORG_PN=MPEG4_P2_MP4_NDSD,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_MULT5_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_HEAAC_L2,"
"http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_HEAAC_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_HEAAC_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT5,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT5_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT5_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT7,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT7_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_AAC_LTP_MULT7_ISO,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L2,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_MPEG1_L3,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AC3,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LTP,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LTP_MULT5,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LTP_MULT7,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_ATRAC3plus,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3L_SD_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L3_SD_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_BSAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L2_CIF30_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_MULT5,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAAC_L2,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_MPEG1_L3,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AC3,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_LTP,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_LTP_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_BSAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_BSAC_MULT5,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_AAC_940,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AMR,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_LTP,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_LTP_520,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_BSAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L1B_QCIF15_HEAAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAACv2_L2,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAACv2_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_MULT5_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_HEAAC_L2,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_HEAAC_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_HEAAC_L2_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_MPEG1_L3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_940_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_MULT5,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_MULT5_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF30_AAC_LTP_MULT5_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_MULT5_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_HEAAC_L2,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_HEAAC_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_HEAAC_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_MPEG1_L3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT5,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT5_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT5_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT7,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT7_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_AAC_LTP_MULT7_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_540_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_LTP,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_LTP_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_AAC_LTP_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_BSAC,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_BL_CIF15_BSAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_BL_CIF15_BSAC_ISO,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AAC,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AAC_LTP,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_HEAAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_HEAAC,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF15_AMR_WBplus,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF30_AMR_WBplus,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AMR,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AMR_WBplus,http-get:*:video/mp4:DLNA.ORG_PN=AVC_3GPP_BL_QCIF15_AMR,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_BSAC_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_BSAC_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_BSAC,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_L1B_QCIF15_AMR,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_L1B_QCIF15_AMR_WBplus,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_HEAACv2_L4_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_HEAACv2_L4_ISO,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_SD_HEAACv2_L4,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_HEAACv2_L4_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_HEAACv2_L4_ISO,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_HEAACv2_L4,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF15_AMR_WBplus_res,\
http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_CIF30_AMR_WBplus_res,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTS,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTSHD_HRA,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTSHD_MA,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_PS_HD_DTSHD,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_SD_EAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_SD_EAC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_EAC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_EAC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_SD_MPEG1_L2_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_SD_MPEG1_L2_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_SD_EU,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_SD_EU_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_SD_EU_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_MP_HD_EAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_MP_HD_EAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_EAC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_EAC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HP_HD_MPEG1_L2_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HP_HD_MPEG1_L2_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_EU,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_EU_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_EU_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_JP_AAC_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_60_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_T,http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_50_AC3_ISO,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_24_AC3_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_DTS_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_DTS_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_DTSHD_HRA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_HD_DTSHD_HRA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_DTSHD_MA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=AVC_TS_DTSHD_MA_ISO,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L1B_QCIF15_HEAACv2,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAACv2,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF30_HEAAC_MPS,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_MPS,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_MPS,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_HEAAC_L4,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_AAC_MPS,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_HEAAC_MPS,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_720p_AAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_HD_1080i_AAC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_AAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_AAC_LTP,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_HEAAC_L2,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_HP_HD_HEAAC_MULT7,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_NDHD,http-get:*:video/3gpp:DLNA.ORG_PN=AVC_3GPP_BL_L12_CIF15_AMR_WBplus,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_AC3_X_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_AC3_X_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_AC3_X_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_60_LPCM_T,\
http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_50_LPCM_T,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=AVC_TS_HD_24_LPCM_T,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L12_CIF15_HEAACv2_350,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_AAC_350,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_CIF15_HEAAC_350,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_MP_SD_AAC_LC,\
http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L31_HD_AAC,http-get:*:video/mp4:DLNA.ORG_PN=AVC_MP4_BL_L32_HD_AAC,\
http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL,http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMALSL_MULT5,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_BASE,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_FULL,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVMED_PRO,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_FULL,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHIGH_PRO,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVHM_BASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPLL_BASE,http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_BASE,\
http-get:*:video/x-ms-wmv:DLNA.ORG_PN=WMVSPML_MP3,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTS,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTSHD_HRA,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTSHD_MA,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_PS_HD_DTSHD,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L1_AC3_ISO,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L1_WMA_ISO,http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L2_AC3_ISO,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_AP_L2_WMA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=VC1_TS_HD_DTS_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_HD_DTS_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=VC1_TS_HD_DTSHD_HRA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_HD_DTSHD_HRA_ISO,http-get:*:video/vnd.dlna.mpeg-tts:DLNA.ORG_PN=VC1_TS_HD_DTSHD_MA_T,\
http-get:*:video/mpeg:DLNA.ORG_PN=VC1_TS_HD_DTSHD_MA_ISO,http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L1_WMA,\
http-get:*:video/x-ms-asf:DLNA.ORG_PN=VC1_ASF_AP_L2_WMA";

#endif

static const char *SinkProtocolInfo = "http-get:*:video/H264:*,http-get:*:video/H261:*,http-get:*:video/H263:*,"\
"http-get:*:video/MPV:*,http-get:*:video/JPEG:*,http-get:*:video/mpeg4-generic:*,http-get:*:video/vc1:*,"\
"http-get:*:video/raw:*,http-get:*:video/x-pn-realvideo:*,http-get:*:video/rtx:*,http-get:*:video/x-ms-asf:*,"\
"http-get:*:video/avi:*,http-get:*:video/x-dv:*,http-get:*:video/x-ms-wmv:*,http-get:*:video/x-ms-wmx:*,http-get:*:"\
"video/x-la-asf:*,http-get:*:video/x-motion-jpeg:*,http-get:*:video/mpeg:*,http-get:*:video/mp2p:*,http-get:*:video/mp2t:*,"\
"http-get:*:video/mpeg2:*,http-get:*:video/mp4:*,http-get:*:video/x-nerodigital-ps:*,http-get:*:video/quicktime:*,"\
"http-get:*:video/x-ms-avi:*,http-get:*:video/x-matroska:*,http-get:*:video/3gpp:*,http-get:*:video/x-msvideo:*,"\
"http-get:*:video/x-sgi-movie:*,http-get:*:video/isivideo:*,http-get:*:video/x-mng:*,http-get:*:video/vnd.rn-realvideo:*,"\
"http-get:*:video/vdo:*,http-get:*:video/vivo:*,http-get:*:video/x-ms-wm:*,http-get:*:video/MP4V-ES:*,http-get:*:video/x-flv:*,"\
"http-get:*:audio/PCMA:*,http-get:*:audio/PCMU:*,http-get:*:audio/x-mpegurl:*,http-get:*:audio/ma1:*,http-get:*:audio/ma2:*,"\
"http-get:*:audio/ma3:*,http-get:*:audio/ma5:*,http-get:*:audio/tsplayer:*,http-get:*:audio/vib:*,http-get:*:audio/nsnd:*,"\
"http-get:*:audio/x-pac:*,http-get:*:audio/x-epac:*,http-get:*:audio/vnd.qcelp:*,http-get:*:audio/x-rmf:*,http-get:*:audio/x-pn-realaudio-plugin:*,http-get:*:audio/voxware:*,http-get:*:audio/x-twinvq-plugin:*,http-get:*:audio/x-twinvq:*,http-get:*:audio/x-ms-wax:*,"\
"http-get:*:audio/x-ms-wmv:*,http-get:*:audio/melody:*,http-get:*:audio/x-mod:*,http-get:*:audio/amr-wb:*,http-get:*:audio/echospeech:*,"\
"http-get:*:audio/x-pn-realaudio:*,http-get:*:audio/x-mpeg:*,http-get:*:audio/x-aiff:*,http-get:*:audio/X-Alpha5:*,http-get:*:audio/3gpp:*,"\
"http-get:*:audio/x-aac:*,http-get:*:audio/x-ac3:*,http-get:*:audio/aiff:*,http-get:*:audio/x-atrac3:*,http-get:*:audio/basic:*,"\
"http-get:*:audio/x-dts:*,http-get:*:audio/midi:*,http-get:*:audio/mp1:*,http-get:*:audio/mp2:*,http-get:*:audio/mpeg:*,http-get:*:audio/mp4:*,http-get:*:audio/x-ogg:*,"\
"http-get:*:audio/wav:*,http-get:*:audio/l16:*,http-get:*:audio/x-ms-wma:*,http-get:*:audio/x-pn-realaudio:*,http-get:*:audio/x-flac:*,"\
"http-get:*:audio/x-scpls:*,http-get:*:audio/mpegurl:*,http-get:*:audio/MPA:*,http-get:*:applicaction/vnd.rn-realmedia:*,http-get:*:applicaction/mp4:*,"\
"http-get:*:applicaction/vnd.ms-asf:*,http-get:*:applicaction/octet-stream:*,http-get:*:text/srt:*,http-get:*:text/ssa:*,"\
"http-get:*:text/srt:*,http-get:*:text/psb:*,http-get:*:text/pjs:*,http-get:*:text/sub:*,http-get:*:text/idx:*,http-get:*:text/dks:*,http-get:*:text/scr:*,http-get:*:text/tts:*,http-get:*:text/vsf:*,http-get:*:text/zeg:*,http-get:*:text/mpl:*,http-get:*:text/bup:*,"\
"http-get:*:text/ifo:*,http-get:*:image/bmp:*,http-get:*:image/x-icon:*,http-get:*:image/gif:*,http-get:*:image/jpeg:*,"\
"http-get:*:image/x-ms-bmp:*,http-get:*:image/png:*,http-get:*:image/x-portable-anymap:*,http-get:*:image/x-portable-pixmap:*,http-get:*:image/x-quicktime:*,"\
"http-get:*:image/tiff:*,rtsp-rtp-udp:*:audio/mpeg:*,rtsp-rtp-udp:*:audio/x-ms-wma:*,rtsp-rtp-udp:*:video/mpeg:*,"\
"rtsp-rtp-udp:*:video/x-ms-wmv:*,rtsp-rtp-udp:*:MPV:*,rtsp-rtp-udp:*:PCMU:*,rtsp-rtp-udp:*:PCMA:*,rtsp-rtp-udp:*:H261:*,"\
"rtsp-rtp-udp:*:MP2T:*,rtsp-rtp-udp:*:H263:*,rtsp-rtp-udp:*:H264:*,rtsp-rtp-udp:*:H264-SVC:*,rtsp-rtp-udp:*:MP2P:*,rtsp-rtp-udp:*:MP1S:*";

#endif

static void ConnectionManager_var_init(void)
{
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 0, "A_ARG_TYPE_AVTransportID", "0", 2);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 1, "A_ARG_TYPE_ConnectionID", "0", 0);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 2, "A_ARG_TYPE_ConnectionManager","0", 0);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 3, "A_ARG_TYPE_ConnectionStatus", "Unknown", 0);//  OK,ContentFormatMismatch,InsufficientBandwidth,UnreliableChannel,Unknown
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 4, "A_ARG_TYPE_Direction", "0", 0);  //Input,Output
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 5, "A_ARG_TYPE_ProtocolInfo", "0", 0);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 6, "A_ARG_TYPE_RcsID", "0", 2);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 7, "CurrentConnectionIDs", "0", 0);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 8, "LastChange", 0, 0);

	#if 0
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 8, "SinkProtocolInfo", "http-get:*:audio/x-ms-wma:*,\
		http-get:*:audio/x-mpegurl:*,http-get:*:audio/mpegurl:*,http-get:*:audio/mp3:*,http-get:*:audio/mpeg:*,http-get:*:audio/mpeg3:*,\
		http-get:*:video/x-ms-avi:*,http-get:*:video/mpeg:*,http-get:*:video/x-ms-wmv:*", 0);
	#endif
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 9, "SinkProtocolInfo", (char*)SinkProtocolInfo, 0);
	upnp_var_init(&upnp_service_table[CONNECTIONMANAGER_SERVICE], 10, "SourceProtocolInfo", 0, 0);
  upnp_service_table[CONNECTIONMANAGER_SERVICE].var_cnt = 11;
}

static void ConnectionManager_action_init(void)
{
	memset(ConnectionManager_action, 0, sizeof(struct upnp_action) * CONNECTIONMANAGER_ACTION_CNT);
	upnp_action_set(&ConnectionManager_action[0], "GetCurrentConnectionIDs", GetCurrentConnectionIDs);
	upnp_action_set(&ConnectionManager_action[1], "GetCurrentConnectionInfo", GetCurrentConnectionInfo);
	upnp_action_set(&ConnectionManager_action[2], "GetProtocolInfo", GetProtocolInfo);
	upnp_action_set(&ConnectionManager_action[3], "PrepareForConnection", PrepareForConnection);
	upnp_action_set(&ConnectionManager_action[4], "ConnectionComplete", ConnectionComplete);

	upnp_service_table[CONNECTIONMANAGER_SERVICE].action = ConnectionManager_action;
	upnp_service_table[CONNECTIONMANAGER_SERVICE].action_cnt = CONNECTIONMANAGER_ACTION_CNT;
}

static void RenderingControl_var_init(void)
{
	int i_value = 0;
	char str_val[11];

	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 0, "A_ARG_TYPE_Channel", "Master", 0);//Master,LF,RF
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 1, "A_ARG_TYPE_InstanceID", 0, 1);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 2, "A_ARG_TYPE_PresetName", "InstallationDefaults", 2);//FactoryDefaults,InstallationDefaults,Vendor defined
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 3, "BlueVideoBlackLevel", 0, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 4, "BlueVideoGain", 0, 3);
	//yhw_vout_getVideoBrightness(&i_value);
	#ifdef NODEFINE_DINGLEI
	i_value = HySDK_Decoder_GetVideoBrightness();
	#endif
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "default brightness is %d\n", i_value);
	sprintf(str_val, "%d", i_value);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 5, "Brightness", str_val, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 6, "ColorTemperature", 0, 3);
	#ifdef NODEFINE_DINGLEI
	i_value = HySDK_Decoder_GetVideoContrast();
	#endif
	//yhw_vout_getVideoContrast(&i_value);
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "default Contrast is %d\n", i_value);
	sprintf(str_val, "%d", i_value);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 7, "Contrast", str_val, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 8, "GreenVideoBlackLevel", 0, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 9, "GreenVideoGain", 0, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 10, "HorizontalKeystone", 0, 4);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 11, "LastChange", 0, 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 12, "Loudness", 0, 5);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 13, "Mute", "0", 5);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 14, "PresetNameList", "FactoryDefaults,InstallationDefaults", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 15, "RedVideoBlackLevel", 0, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 16, "RedVideoGain", 0, 3);
	#ifdef NODEFINE_DINGLEI
	i_value = HySDK_Decoder_GetVideoSharpness();
	#endif
	//yhw_vout_getVideoSharpness(&i_value);
	sprintf(str_val, "%d", i_value);
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "default Sharpness is %d\n", i_value);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 17, "Sharpness", str_val, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 18, "VerticalKeystone", 0, 4);
	//yhw_aout_getVolume(&i_value);
	#ifdef NODEFINE_DINGLEI
	i_value = HySDK_Decoder_GetVolume();	//By Chenwei
	#endif
	sprintf(str_val, "%d", i_value);
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "default Volume is %d\n", i_value);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 19, "Volume", str_val, 3);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 20, "VolumeDB", 0, 4);

	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 21, "ZoomMAX", "400", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 22, "ZoomMIN", "1", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 23, "VolumeMAX", "100", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 24, "VolumeMIN", "1", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 25, "ClosedCaptioning", "en", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 26, "AudioTrackSelection", "None", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 27, "Rotation", "0", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 28, "X_Philips.com_AmbiLight", "Normal", 0);
	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 29, "Mute channel", "Master", 0);
 	upnp_var_init(&upnp_service_table[RENDERINGCONTROL_SERVICE], 30, "Volume channel", "Master", 0);

	upnp_service_table[RENDERINGCONTROL_SERVICE].var_cnt = 31;


}

static void RenderingControl_action_init(void)
{
	memset(RenderingControl_action, 0, sizeof(struct upnp_action) * RENDERINGCONTROL_ACTION_CNT);
	upnp_action_set(&RenderingControl_action[0], "GetBlueVideoBlackLevel", GetBlueVideoBlackLevel);
	//upnp_action_set(&RenderingControl_action[1], "GetBlueVideoGain", GetBlueVideoGain);
	upnp_action_set(&RenderingControl_action[2], "GetBrightness", GetBrightness);
	upnp_action_set(&RenderingControl_action[3], "GetColorTemperature", GetColorTemperature);
	upnp_action_set(&RenderingControl_action[4], "GetContrast", GetContrast);
	upnp_action_set(&RenderingControl_action[5], "GetGreenVideoBlackLevel", GetGreenVideoBlackLevel);
	//upnp_action_set(&RenderingControl_action[6], "GetGreenVideoGain", GetGreenVideoGain);
	//upnp_action_set(&RenderingControl_action[7], "GetHorizontalKeystone", GetHorizontalKeystone);
	//upnp_action_set(&RenderingControl_action[8], "GetLoudness", GetLoudness);
	upnp_action_set(&RenderingControl_action[9], "GetMute", GetMute);
	upnp_action_set(&RenderingControl_action[10], "GetRedVideoBlackLevel", GetRedVideoBlackLevel);
	//upnp_action_set(&RenderingControl_action[11], "GetRedVideoGain", GetRedVideoGain);
	upnp_action_set(&RenderingControl_action[12], "GetSharpness", GetSharpness);
	//upnp_action_set(&RenderingControl_action[13], "GetVerticalKeystone", GetVerticalKeystone);
	upnp_action_set(&RenderingControl_action[14], "GetVolume", GetVolume);
	//upnp_action_set(&RenderingControl_action[15], "GetVolumeDB", GetVolumeDB);
	//upnp_action_set(&RenderingControl_action[16], "GetVolumeDBRange", GetVolumeDBRange);
	upnp_action_set(&RenderingControl_action[17], "ListPresets", ListPresets);
	upnp_action_set(&RenderingControl_action[18], "SelectPreset", SelectPreset);
	upnp_action_set(&RenderingControl_action[19], "SetBlueVideoBlackLevel", SetBlueVideoBlackLevel);
	//upnp_action_set(&RenderingControl_action[20], "SetBlueVideoGain", SetBlueVideoGain);
	upnp_action_set(&RenderingControl_action[21], "SetBrightness", SetBrightness);
	upnp_action_set(&RenderingControl_action[22], "SetColorTemperature", SetColorTemperature);
	upnp_action_set(&RenderingControl_action[23], "SetContrast", SetContrast);
	upnp_action_set(&RenderingControl_action[24], "SetGreenVideoBlackLevel", SetGreenVideoBlackLevel);
	//upnp_action_set(&RenderingControl_action[25], "SetGreenVideoGain", SetGreenVideoGain);
	//upnp_action_set(&RenderingControl_action[26], "SetHorizontalKeystone", SetHorizontalKeystone);
	//upnp_action_set(&RenderingControl_action[27], "SetLoudness", SetLoudness);
	upnp_action_set(&RenderingControl_action[28], "SetMute", SetMute);
	upnp_action_set(&RenderingControl_action[29], "SetRedVideoBlackLevel", SetRedVideoBlackLevel);
	//upnp_action_set(&RenderingControl_action[30], "SetRedVideoGain", SetRedVideoGain);
	upnp_action_set(&RenderingControl_action[31], "SetSharpness", SetSharpness);
	//upnp_action_set(&RenderingControl_action[32], "SetVerticalKeystone", SetVerticalKeystone);
	upnp_action_set(&RenderingControl_action[33], "SetVolume", SetVolume);
	upnp_action_set(&RenderingControl_action[34], "GetAllowedTransforms", GetAllowedTransforms);
	upnp_action_set(&RenderingControl_action[35], "SetTransforms", SetTransforms);

	//upnp_action_set(&RenderingControl_action[34], "SetVolumeDB", SetVolumeDB);
	upnp_service_table[RENDERINGCONTROL_SERVICE].action = RenderingControl_action;
	upnp_service_table[RENDERINGCONTROL_SERVICE].action_cnt = RENDERINGCONTROL_ACTION_CNT;
}

static void X_CTC_Subscribe_var_init(void);
static void X_CTC_Subscribe_action_init(void);
static void X_CTC_Subscribe_var_init(void)
{
	upnp_var_init(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], 0, "ContentType", "1", 0);//Master,LF,RF
	upnp_var_init(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], 1, "ProductId", 0, 0);
	upnp_var_init(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], 2, "ContentId", 0, 0);//FactoryDefaults,InstallationDefaults,Vendor defined
	upnp_var_init(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], 3, "ServiceId", 0, 0);
	upnp_var_init(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], 4, "ColumnId", 0, 0);
	upnp_var_init(&upnp_service_table[X_CTC_SUBSCRIBE_SERVICE], 5, "LastChange", 0, 0);
	upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].var_cnt = 6;

}
static void X_CTC_Subscribe_action_init(void)
{
	memset(X_CTC_SUBSCRIBE_action, 0, sizeof(struct upnp_action) * X_CTC_SUBSCRIBE_ACTION_CNT);
	upnp_action_set(&X_CTC_SUBSCRIBE_action[0], "GetProductInfo", X_CTC_GetProductInfo);
	upnp_action_set(&X_CTC_SUBSCRIBE_action[1], "Order", X_CTC_Order);
	upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].action = X_CTC_SUBSCRIBE_action;
	upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].action_cnt = X_CTC_SUBSCRIBE_ACTION_CNT;

}

static int render_state_table_init( IN char *DescDocURL )
{
  IXML_Document *DescDoc = NULL;
  int ret = UPNP_E_SUCCESS;
  char* servid_ctrl = NULL;
  char* evnturl_ctrl = NULL;
  char* ctrlurl_ctrl = NULL;
  char* servid_pict = NULL;
  char* evnturl_pict = NULL;
  char* ctrlurl_pict = NULL;
  char* udn = NULL;
  int i;

  //Download description document
  if( UpnpDownloadXmlDoc( DescDocURL, &DescDoc ) != UPNP_E_SUCCESS ) {
    UpnpUtil_Print( "render_state_table_init -- Error Parsing %s\n", DescDocURL );
    ret = UPNP_E_INVALID_DESC;
    goto error_handler;
  }
  udn = UpnpUtil_GetFirstDocumentItem( DescDoc, "UDN" );

  for(i = 0; i < RENDER_SERVICE_CNT; i ++){
    memset(&upnp_service_table[i], 0, sizeof(struct upnp_service));
  }
  /*
     Find the AVTransport Service identifiers
     */
  if (!UpnpUtil_FindAndParseService( DescDoc, DescDocURL,
      render_service_type[AVTRANSPORT_SERVICE], &servid_ctrl, &evnturl_ctrl, &ctrlurl_ctrl ) ) {
    UpnpUtil_Print( "render_state_table_init -- Error: Could not find Service: %s\n", render_service_type[AVTRANSPORT_SERVICE] );
    ret = UPNP_E_INVALID_DESC;
    goto error_handler;
  }


  //set AVTransport control service table
  AVTransport_var_init();
  AVTransport_action_init();
  strcpy(upnp_service_table[AVTRANSPORT_SERVICE].service_udn, udn);
  strcpy(upnp_service_table[AVTRANSPORT_SERVICE].service_type,
    render_service_type[AVTRANSPORT_SERVICE]);
  strcpy(upnp_service_table[AVTRANSPORT_SERVICE].service_id, servid_ctrl);

  //SetServiceTable( AVTRANSPORT_SERVICE, udn, servid_ctrl, render_service_type[AVTRANSPORT_SERVICE], &upnp_service_table[AVTRANSPORT_SERVICE] );

  /*
     Find the connection Manager Service identifiers
     */
  if( !UpnpUtil_FindAndParseService( DescDoc, DescDocURL,
      render_service_type[CONNECTIONMANAGER_SERVICE], &servid_ctrl, &evnturl_pict, &ctrlurl_pict ) ) {
    UpnpUtil_Print( "render_state_table_init -- Error: Could not find Service: %s\n", render_service_type[CONNECTIONMANAGER_SERVICE] );
    ret = UPNP_E_INVALID_DESC;
    goto error_handler;
  }

  //set ConnectionManager control service table
  ConnectionManager_var_init();
  ConnectionManager_action_init();
  strcpy(upnp_service_table[CONNECTIONMANAGER_SERVICE].service_udn, udn);
  strcpy(upnp_service_table[CONNECTIONMANAGER_SERVICE].service_type, render_service_type[CONNECTIONMANAGER_SERVICE]);
  strcpy(upnp_service_table[CONNECTIONMANAGER_SERVICE].service_id, servid_ctrl);
  /*
     Find the Render Control Service identifiers
     */
  if( !UpnpUtil_FindAndParseService( DescDoc, DescDocURL, render_service_type[RENDERINGCONTROL_SERVICE], &servid_ctrl, &evnturl_ctrl, &ctrlurl_ctrl ) ) {
    UpnpUtil_Print( "render_state_table_init -- Error: Could not find Service: %s\n", render_service_type[RENDERINGCONTROL_SERVICE] );
    ret = UPNP_E_INVALID_DESC;
    goto error_handler;
  }

  //set Rendering control service table
  RenderingControl_var_init();
  RenderingControl_action_init();
  strcpy(upnp_service_table[RENDERINGCONTROL_SERVICE].service_udn, udn);
  strcpy(upnp_service_table[RENDERINGCONTROL_SERVICE].service_type,
    render_service_type[RENDERINGCONTROL_SERVICE]);
  strcpy(upnp_service_table[RENDERINGCONTROL_SERVICE].service_id, servid_ctrl);

  /*
     Find the X_CTC Subscribe Service identifiers
     */
  if( !UpnpUtil_FindAndParseService( DescDoc, DescDocURL, render_service_type[X_CTC_SUBSCRIBE_SERVICE], &servid_ctrl, &evnturl_ctrl, &ctrlurl_ctrl ) ) {
    UpnpUtil_Print( "render_state_table_init -- Error: Could not find Service: %s\n", render_service_type[X_CTC_SUBSCRIBE_SERVICE] );
    ret = UPNP_E_INVALID_DESC;
    goto error_handler;
  }
  //set X_CTC Subscribe service table
  X_CTC_Subscribe_var_init();
  X_CTC_Subscribe_action_init();
  strcpy(upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].service_udn, udn);
  strcpy(upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].service_type,
    render_service_type[X_CTC_SUBSCRIBE_SERVICE]);
  strcpy(upnp_service_table[X_CTC_SUBSCRIBE_SERVICE].service_id, servid_ctrl);
error_handler:

  //clean up
  if( udn )
    value_free( udn );
  if( servid_ctrl )
    value_free( servid_ctrl );
  if( evnturl_ctrl )
    value_free( evnturl_ctrl );
  if( ctrlurl_ctrl )
    value_free( ctrlurl_ctrl );
  if( servid_pict )
    value_free( servid_pict );
  if( evnturl_pict )
    value_free( evnturl_pict );
  if( ctrlurl_pict )
    value_free( ctrlurl_pict );
  if( DescDoc )
    ixmlDocument_free( DescDoc );

  return ( ret );
}


static int render_evt_callback_handler( Upnp_EventType EventType, void *Event,  void *Cookie )
{
  HT_DBG_FUNC_START(HT_MOD_DMR, HT_BIT_MANY, EventType, NULL);
  //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%s():action acur...\n", __FUNCTION__);
  switch ( EventType ) {
  case UPNP_EVENT_SUBSCRIPTION_REQUEST:
    RenderHandleSubscriptionRequest( ( struct Upnp_Subscription_Request * )Event );
    break;
  case UPNP_CONTROL_GET_VAR_REQUEST:
    RenderHandleGetVarRequest( ( struct Upnp_State_Var_Request * )Event );
    break;
  case UPNP_CONTROL_ACTION_REQUEST:
    RenderHandleActionRequest( ( struct Upnp_Action_Request * )Event );
    break;
    /*ignore these cases, since this is not a control point */
  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
  case UPNP_DISCOVERY_SEARCH_RESULT:
  case UPNP_DISCOVERY_SEARCH_TIMEOUT:
  case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
  case UPNP_CONTROL_ACTION_COMPLETE:
  case UPNP_CONTROL_GET_VAR_COMPLETE:
  case UPNP_EVENT_RECEIVED:
  case UPNP_EVENT_RENEWAL_COMPLETE:
  case UPNP_EVENT_SUBSCRIBE_COMPLETE:
  case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
    break;
  default:
    UpnpUtil_Print( "Error in render_evt_callback_handler: unknown event type %d\n", EventType );
  }

  /*	Print a summary of the event received 	*/
  //	UpnpUtil_PrintEvent( EventType, Event );

  HT_DBG_FUNC_END(EventType, NULL);
  return ( 0 );
}

int  g_FocusAVTransport = 0;
static void upnp_set_RunningAVTransportHandle(int handle)
{
	g_FocusAVTransport = handle;
}
#if 0
static void avTransportStateCall(int eventkind, int value)
{
	if(eventkind == SYSTEM){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "will_debug key value is %d\n", value);
		if(value == STREAM_STATUS_STOP)
		{
			upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "STOPPED");
			if(g_FocusAVTransport)
			{
				HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "\n\n\naaaaaaaaaaaaaaaa Go to End g_FocusAVTransport = %d\n", g_FocusAVTransport);
				g_Dlna_EventList_Put((void *)g_FocusAVTransport, "{\"eventType\":\"DLNA_MSG_DMR_AVEND\"}");
			}
		}
		else if(value == STREAM_STATUS_FAILURE)
		{
			upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "STOPPED");
			if(g_FocusAVTransport)
			{
				HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "\n\n\naaaaaaaaaaaaaaaa Go Fail\n");
				g_Dlna_EventList_Put((void *)g_FocusAVTransport, "{\"eventType\":\"DLNA_MSG_DMR_AVFAIL\"}");
			}
		}
		else if(value == STREAM_STATUS_PLAY)
		{
			upnp_var_set(&upnp_service_table[AVTRANSPORT_SERVICE], "TransportState", "PLAYING");
			if(g_FocusAVTransport)
			{
				HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "\n\n\naaaaaaaaaaaaaaaa Start Play g_FocusAVTransport = %d\n", g_FocusAVTransport);
				g_Dlna_EventList_Put((void *)g_FocusAVTransport, "{\"eventType\":\"DLNA_MSG_DMR_AVSTART\"}");
			}
		}
	}
}
#endif
#define RENDER_SERVICE_DOC "AVRenderService.xml"
#define DESC_URL_SIZE 64

static inline int mac_fmt_change(const char *src_mac, char *dst_mac)
{
	int src_len = strlen(src_mac);
	int i;

	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "mac is %s\n", src_mac);
	for(i = 0; i < src_len; i++){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%c", src_mac[i]);
		if(src_mac[i] != ':'){
			dst_mac[0] = src_mac[i];
			dst_mac ++;
		}
	}
	dst_mac[0] = 0;
	return 0;
}

#include "RenderService.h"
#include <errno.h>
#if 0 
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netinet/in_systm.h>
 #include <netinet/ip.h>
 #include <netinet/ip_icmp.h>
static int upnp_getMACAddress(char *MACAddress) 
{ 
	struct ifreq ifr; 
	int so; 
	unsigned char mac[8]; 

	if (!MACAddress) 
		return -1; 
	so = socket(AF_INET, SOCK_DGRAM, 0);
	if (so < 0){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "get mac sockfd err\n");
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name,"eth0"); 
	ifr.ifr_addr.sa_family = AF_INET;
	if( ioctl(so , SIOCGIFHWADDR, ifr) < 0){
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "get mac ioctl err errno1  = %d\n",errno);
		close(so); 
		return -1; 
	}

	/*bzero (&(ifr.ifr_addr), sizeof (struct sockaddr_in)); 
	strcpy(ifr.ifr_name,"eth0"); 
	((struct sockaddr_in *) &ifr.ifr_addr)->sin_family = AF_INET; 
	if (ioctl(so, SIOCGIFHWADDR, &ifr) < 0) 
	{ 	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "mac err1\n");

		close(so); 
		return -1; 
	} */
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 8); 
	sprintf(MACAddress,"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]); 
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "mac %s\n", MACAddress);
//*MACAddress = macaddr; 
	close(so); 
	return 0; 
} 
 #endif
static int render_service_create(char *friend_name)
{
	FILE * fp;
	char *dst_buf = NULL;
	char mac[128] = {0};
	//char *mac = NULL;
	//char local_mac[13] = "1a2b3c4d5e6f";	//By Chenwei
	char local_mac[12 + 1] = {0};		//By Chenwei
	int ret = -1;
	char *local_name = NULL; //friend_name + mac;
	int local_name_len = 0;
	std::string android_mac = "00:00:00:00:00:00";
	render_track();
	//yos_net_getMACAddress(&mac);
	//mac = "00:07:67:68:3e";//HySDK_STB_GetMacAddress();
//	HySDK_STB_GetMacAddress(mac);
	//txt.push_back(std::make_pair("deviceid", iface != NULL ? iface->GetMacAddress() : "FF:FF:FF:FF:FF:F2"));
	//CNetworkInterface* iface = CApplication::getInstance().getNetwork().GetFirstConnectedInterface();
	//android_mac = iface->GetMacAddress();
	//upnp_getMACAddress(mac);
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "mac %s\n", DMR_mac_addr);

	mac_fmt_change(DMR_mac_addr, local_mac);
	local_name_len = strlen(friend_name) + strlen(local_mac) + strlen("()") ;
	local_name = (char *)malloc(local_name_len + 1);
	sprintf(local_name, "%s(%s)", friend_name, local_mac);
	dst_buf = (char *)malloc(strlen(av_render_service) + local_name_len + strlen(local_mac) * 5 + 1);
	sprintf(dst_buf, av_render_service, local_name, local_mac, local_mac, local_mac ,local_mac,local_mac);
	#if 1
	char filepath[128] = {0};
	sprintf(filepath,"%s%s",DMR_path,RENDER_CFG_FILE);
	fp = fopen(filepath, "w+");
	if(fp){
		fwrite(dst_buf, strlen(dst_buf), 1, fp);
		fclose(fp);
		ret = 0;
	}
	else
	{
					HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "fp NULL %s %s, errno=%d\n",DMR_path,filepath, errno);


	}
	#else
	{
		int fd = open(WEB_PATH RENDER_CFG_FILE, O_CREAT | O_WRONLY | O_SYNC );
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "\n open render.xml = %d \n", fd);
		if(fd>0)
		{
			write(fd, dst_buf, strlen(dst_buf));
			close(fd);
			ret = 0;
		}
	}
	#endif
	if(dst_buf)
		value_free(dst_buf);
	if(local_name)
		value_free(local_name);
	return ret;
}

static int  service_file_check()
{
	FILE *fp;

	render_track();
	fp = fopen(WEB_PATH RENDER_CFG_FILE, "r");
	if(fp){
		fclose(fp);
		char render_name[128] = {0};
		av_render_name_get(render_name,  128);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY,"old name = %s\n",render_name);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY,"new name = %s\n",DMR_name);
		if(strcmp(DMR_name,render_name)!=0){
			av_render_name_set(DMR_name);
		}
		return 0;
	} else {
		render_track();
		return render_service_create(DMR_name);
	}	
}

int av_render_name_get(char *render_name,  int len)
{
	FILE *fp;
	IXML_Document* doc = NULL;
	int rc;
	char *value = NULL;
	
	render_track();
	fp = fopen(WEB_PATH RENDER_CFG_FILE, "r");
	if(fp){
	render_track();
		fclose(fp);
		rc = ixmlLoadDocumentEx (WEB_PATH RENDER_CFG_FILE, &doc);
		if (rc != 0) {
			render_msg("error to get %s\n", WEB_PATH RENDER_CFG_FILE);
			return -1;
		}		
		value = UpnpUtil_GetFirstDocumentItem( doc, "friendlyName" );
		if(value){
			strncpy(render_name, value, min(len, strlen(value)));
			value_free(value);
			return 0;
		}
		ixmlDocument_free (doc);
	}
	return -1;
}

int av_render_name_set(char *render_name)
{
	FILE *fp;
	IXML_Document* doc = NULL;
	int rc;
//	char *value = NULL;
	char *txt = NULL;
	
	render_track();
	fp = fopen(WEB_PATH RENDER_CFG_FILE, "r");
	if(fp){
	render_track();
		fclose(fp);
		rc = ixmlLoadDocumentEx (WEB_PATH RENDER_CFG_FILE, &doc);
		if (rc != 0) {
			render_msg("error to get %s\n", WEB_PATH RENDER_CFG_FILE);
			return -1;
		}		
		UpnpUtil_SetFirstDocumentItem(doc, "friendlyName", render_name);
		txt = ixmlPrintDocument(doc);
	#if 0
		fp = fopen(WEB_PATH RENDER_CFG_FILE, "w");
		fwrite(txt, strlen(txt), 1, fp);
		fclose(fp);
	#else
	{
		int fd = open(WEB_PATH RENDER_CFG_FILE, O_WRONLY | O_SYNC|O_TRUNC);
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "\n open render.xml = %d \n", fd);
		if(fd>0)
		{
			write(fd, txt, strlen(txt));
			close(fd);
		}
	}
	#endif
		if(txt)
			value_free(txt);
		ixmlDocument_free (doc);
		return 0;
	}
	return -1;
}

static int av_render_start( char *ip_address, unsigned short port, char *desc_doc_name, char *web_dir_path, print_string pfun )
{
	int ret = UPNP_E_SUCCESS;
	char desc_doc_url[DESC_URL_SIZE];

	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "will_debug %s\n", __FUNCTION__);
	if (service_file_check() != 0) {
		HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "service_file_check err %s\n", __FUNCTION__);

		return -1;
	}
	ithread_mutex_init( &render_mux, NULL );
	ithread_mutex_init(&var_mux, NULL);
	UpnpUtil_Initialize( pfun );
	HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY,  "Initializing UPnP Sdk with\n" "\tipaddress = %s port = %u\n", ip_address, port ); //By Chenwei
#ifdef RENDER_STANDALONE
	if( ( ret = UpnpInit( ip_address, port ) ) != UPNP_E_SUCCESS ) {
		UpnpUtil_Print( "Error with UpnpInit -- %d\n", ret );
		UpnpFinish();
		return ret;
	}
#endif
	if( ip_address == NULL ) {
		ip_address = UpnpGetServerIpAddress();
	}
	port = UpnpGetServerPort();
	UpnpUtil_Print("UPnP Initialized\n"	"\tipaddress= %s port = %u\n", ip_address, port );
	if( desc_doc_name == NULL ) {
		desc_doc_name = RENDER_SERVICE_DOC;
	}
	if( web_dir_path == NULL ) {
		web_dir_path = WEB_PATH;
	}
	snprintf( desc_doc_url, DESC_URL_SIZE, "http://%s:%d/%s", ip_address, port, desc_doc_name );
	UpnpUtil_Print( "Specifying the webserver root directory -- %s\n", web_dir_path );
#ifdef RENDER_STANDALONE
	if( ( ret = UpnpSetWebServerRootDir( web_dir_path ) ) != UPNP_E_SUCCESS ) {
		UpnpUtil_Print( "Error specifying webserver root directory -- %s: %d\n",  web_dir_path, ret );
		UpnpFinish();
		return ret;
	}
#endif
	UpnpUtil_Print( "Registering the RootDevice\n"   "\t with desc_doc_url: %s\n", desc_doc_url );
	if( ( ret = UpnpRegisterRootDevice( desc_doc_url, render_evt_callback_handler, &render_handle, &render_handle ) ) != UPNP_E_SUCCESS ) {
		UpnpUtil_Print( "Error registering the rootdevice : %d\n", ret );
#ifdef RENDER_STANDALONE
		UpnpFinish();
#endif
		return ret;
	} /*else {
		UpnpUtil_Print("RootDevice Registered\n" "Initializing State Table\n");
		render_state_table_init( desc_doc_url );
		UpnpUtil_Print("State Table Initialized\n");
		if( ( ret = UpnpSendAdvertisement( render_handle, advr_expire ) ) != UPNP_E_SUCCESS ) {
			UpnpUtil_Print( "Error sending advertisements : %d\n", ret );
			UpnpFinish();
			return ret;
		}
		UpnpUtil_Print("Advertisements Sent\n");
	}*/
	UpnpUnRegisterRootDevice(render_handle);
	if( ( ret = UpnpRegisterRootDevice( desc_doc_url, render_evt_callback_handler, &render_handle, &render_handle ) ) != UPNP_E_SUCCESS ) {
		UpnpUtil_Print( "Error registering the rootdevice : %d\n", ret );
		#ifdef RENDER_STANDALONE
		UpnpFinish();
		#endif
		return ret;
	} else {
		UpnpUtil_Print("RootDevice Registered\n" "Initializing State Table\n");
		render_state_table_init( desc_doc_url );
		UpnpUtil_Print("State Table Initialized\n");
		if( ( ret = UpnpSendAdvertisement( render_handle, advr_expire ) ) != UPNP_E_SUCCESS ) {
			UpnpUtil_Print( "Error sending advertisements : %d\n", ret );
#ifdef RENDER_STANDALONE
			UpnpFinish();
#endif
			return ret;
		}
		UpnpUtil_Print("Advertisements Sent\n");
	}

	return UPNP_E_SUCCESS;
}

static int av_render_stop()
{
  upnp_set_RunningAVTransportHandle(0);
  UpnpUnRegisterRootDevice( render_handle );
#ifdef RENDER_STANDALONE
  UpnpFinish();
#endif
  UpnpUtil_Finish();
  ithread_mutex_destroy( &render_mux );
  return UPNP_E_SUCCESS;
}
#if 0
//removed by teddy. 2013.08.16 Fri 15:45:52
static int net_info_get(char *interface, unsigned int *addr, unsigned char *mac)
{
  int fd;
  struct ifreq ifr;
  struct sockaddr_in *our_ip;

  memset(&ifr, 0, sizeof(struct ifreq));
  if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
  {
    render_msg("socket failed!: %s\n", strerror(errno));
    return -1;
  }

  ifr.ifr_addr.sa_family = AF_INET;
  strcpy(ifr.ifr_name, interface);

  if(addr){
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
      our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
      *addr = our_ip->sin_addr.s_addr;
      //HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "%s (our ip) = %s %d\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr), *addr);
    } else {
      render_msg("SIOCGIFADDR failed, is the interface up and configured?: %s\n", strerror(errno));
      close(fd);
      return -1;
    }
  }
  if(mac){
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
      memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
      //printe("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
      render_msg("SIOCGIFHWADDR failed!: %s", strerror(errno));
      close(fd);
      return -1;
    }
  }
  close(fd);
  return 0;
}
#endif
static int msg_out_enable = 0;
static void msg_print( const char *string )
{
	if(msg_out_enable)
		puts(string);
}
void render_dbg_msg_enable(int enable)
{
	msg_out_enable = enable;
}

//static pthread_t render_pid;
static char ip_addr[16] = {0};
static int upnp_port = 12345;
static int render_valid = 0;

static int _render_start()
{
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "will_debug %s\n", __FUNCTION__);
  render_track();
//  struct in_addr ip;

  render_dbg_msg_enable(1);
  /*
     if(ip_addr[0] == 0 || strlen(ip_addr) < 7){
     render_track();
     if(net_info_get("eth0", &ip.s_addr, NULL) != 0){
     render_msg("fail to get ip address\n");
     return -1;
     } else {
     render_msg("ip addr is %s\n", inet_ntoa(ip));
     }
     strcpy(ip_addr, inet_ntoa(ip));
     }
     render_track();
  */
  if(av_render_start(ip_addr, upnp_port, NULL, NULL, msg_print) == 0){
    render_valid = 1;
  }
  return 0;
}

int render_start(char *ip, int port)
{
  HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "will_debug %s\n", __FUNCTION__);
  if(ip){
    memcpy(ip_addr, ip, 15);
    ip_addr[15] = 0;
    if(port > 0)
      upnp_port = port;
    HT_DBG_PRINTF(HT_MOD_DMR, HT_BIT_MANY, "will_debug %s\n", __FUNCTION__);
    _render_start();
    //pthread_create(&render_pid, NULL, _render_start, NULL);
    return 0;
  }
  render_msg("ip addr is NULL\n");
  return -1;
}
void render_stop(void)
{
  if(render_valid){
    render_valid = 0;
    media_stop();
#ifdef NODEFINE_DINGLEI
    render_ui_close();
#endif
    render_track();
    av_render_stop();
#if 1
#ifdef NODEFINE_DINGLEI
    image_process_destroy();
#endif
    render_track();
    memset(ip_addr, 0, 15);
    render_track();
    upnp_port = 12345;//default
    render_track();
#endif
  }
}

int Dmr_Init(char *cfg)
{
  return 0;
}
int Raw_Dmr_Init(const char *path,const char *name,const char *mac_addr)
{
	if(!name && !path)
		return 0;
	  HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_KEY, "DMR_path =%s DMR_name = %s \n",DMR_path,DMR_name);

	strncpy(DMR_name, name, min(128, strlen(name)));
	strncpy(DMR_path, path, min(128, strlen(path)));
	strncpy(DMR_mac_addr, mac_addr, min(128, strlen(mac_addr)));

	return 0;
}
int Dmr_Start(DMREVRNT_CALLBACK_CPP Callback)
{	//return 0;
  HT_DBG_FUNC_START(HT_MOD_IPC, HT_BIT_KEY,0, 0);
  //int ret = -1;
  int port = UpnpGetServerPort();
  char *ip = UpnpGetServerIpAddress();
  HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_KEY, "ip = %s \n", ip);
  if(render_start(ip, port)) {
    return -1;
  }
  HT_DBG_PRINTF(HT_MOD_IPC, HT_BIT_KEY, "ip = %s \n", ip);
  if(Callback)
    DMREvent_Callback = Callback;
  HT_DBG_FUNC_END(0, NULL);

  return 0;
}
int Dmr_Stop(void)
{
  render_stop();
  return 0;
}

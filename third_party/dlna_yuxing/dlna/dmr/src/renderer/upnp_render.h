#ifndef UPNP_RENDER_H
#define UPNP_RENDER_H

#include "ixml.h"
//#include "upnp_port.h"
/******************************************************************************
 * upnp_action
 *
 * Description:
 *       Prototype for all actions. For each action that a service
 *       implements, there is a corresponding function with this prototype.
 *       Pointers to these functions, along with action names, are stored
 *       in the service table. When an action request comes in the action
 *       name is matched, and the appropriate function is called.
 *       Each function returns UPNP_E_SUCCESS, on success, and a nonzero
 *       error code on failure.
 *
 * Parameters:
 *
 *    IXML_Document * request - document of action request
 *    IXML_Document **out - action result
 *    char **errorString - errorString (in case action was unsuccessful)
 *
 *****************************************************************************/

typedef int (*upnp_xml_action) (IXML_Document *request, IXML_Document **out,  char **errorString);

#define UDN_LEN 256
#define SERVICE_ID_LEN UDN_LEN
#define SERVICE_TYPE_LEN UDN_LEN
#define SERVICE_VAR_MAX 128
#define SERVICE_ACTION_MAX 128

struct upnp_var{
	char *var_name;
	char *var_value; //所有数据最终转为字符串保存
	char var_type;//0:string;1:ui4(unsigned int);2:i4(int);3:ui2(unsigned short);4:i2(short);5:boolean
};

struct upnp_action{
	char *action_name;
	upnp_xml_action callback;
};


struct upnp_service {

  char service_udn[UDN_LEN]; /* Universally Unique Device Name */
  char service_id[SERVICE_ID_LEN];
  char service_type[SERVICE_TYPE_LEN];
  char *var_name[SERVICE_VAR_MAX];
  char *var_value[SERVICE_VAR_MAX];
  char var_type[SERVICE_VAR_MAX];
  int var_cnt;
  struct upnp_action *action;
  int action_cnt;
};

#if defined(ANDROID)
#define WEB_PATH "/cache"
#elif defined(WIN32)
#define WEB_PATH "F:\\svn_beijing\\webkit-r55397\\Trunk\\rhino\\third_party\\dlna_yuxing\\dlnawebroot\\dlna\\"
#else
#define WEB_PATH "/root/dlna"
#endif
#define RENDER_CFG_FILE "/AVRenderService.xml"

//typedef void (*DMREVRNT_CALLBACK)(int eventID, void *eventArg, void *arg);
/*
int Raw_Dmr_Init(void);
int Raw_Dmr_Start(DMREVRNT_CALLBACK Callback);
int Raw_Dmr_Stop(void);
*/
int
UpnpAddToActionResponsedinglei( INOUT IXML_Document ** ActionResponse,
                         IN const char *ActionName,
                         IN const char *ServType,
                         IN const char *ArgName,
                         IN const char *ArgValue,
                         IN const char *ArgName_ch,
                         IN const char *ArgValue_ch );

int
UpnpAddToActionResponsedinglei_1( INOUT IXML_Document ** ActionResponse,
                         IN const char *ActionName,
                         IN const char *ServType,
                         IN const char *ArgName_ch1,
                         IN const char *ArgValue_ch1 );
int
UpnpAddToActionResponsedinglei2( INOUT IXML_Document ** ActionResponse,
                         IN const char *ActionName,
                         IN const char *ServType,
                         IN const char *ArgName,
                         IN const char *ArgValue,
                         IN const char *ArgName_ch,
                         IN const char *ArgValue_ch );

#endif

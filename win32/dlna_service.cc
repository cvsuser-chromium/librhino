#if 0
#define LOG_NDEBUG 0
#define LOG_TAG "DLNAService"
#include "rhino_export.h"
#include "base/bind.h"
#include "base/logging.h"
#include "base/at_exit.h"
#include "base/strings/stringprintf.h"
#include "base/string_util.h"
#include "base/strings/string_number_conversions.h"

#include "dlna_service.h"

#include "dlna_api.h"
#include "dlna_api_cpp.h"
#include "hitTime.h"
#include <upnp.h>
#include <iostream>
/*
 *  bind Parcel message always are int32 and string16.
 */
namespace {
base::AtExitManager* g_at_exit_manager = NULL;
dlna::DLNAService* g_dlna_service = NULL;
const char* g_web_dir_path = "F:\\svn_beijing\\webkit-r55397\\Trunk\\rhino\\third_party\\dlna_yuxing\\dlnawebroot\\dlna";
static int s_dlna_callback(enum_DlnaCallback eType, int value, char *str, void *user)
{
  int ret = -1;
  LOG(INFO) << __FUNCTION__ << ": " << str;

  HT_DBG_FUNC_START(HT_MOD_APP, HT_BIT_FEW, eType, str);

  //TODO

  HT_DBG_FUNC_END( value, 0);
  return ret;
}

static int s_dlna_event_handler(enum_DlnaEvent type,int handle, char *str)
{
  LOG(INFO) << __FUNCTION__ << " type=["<< type << "]: " << str;

  return 0;
}
}
#include "base/threading/thread.h"

namespace TestDLNA{
class DLNAMainThread : public base::Thread {
public:
  explicit DLNAMainThread()
      : base::Thread("DLNA_TestThread"){
  }

  virtual ~DLNAMainThread() {
    Stop();
  }
  static base::MessageLoop* MessageLoop();
protected:
  virtual void Init() OVERRIDE {
    LOG(INFO) << __FUNCTION__;
  }

  virtual void CleanUp() OVERRIDE {
    LOG(INFO) << __FUNCTION__;
  }

private:
  std::string channel_id_;
  DISALLOW_COPY_AND_ASSIGN(DLNAMainThread);
  static DLNAMainThread* testThread;
};
DLNAMainThread* DLNAMainThread::testThread = 0;
base::MessageLoop* DLNAMainThread::MessageLoop()
{
  if (!testThread){
    testThread = new DLNAMainThread();
    testThread->Start();
  }
  LOG(INFO) << "DLNAMainThread::message_loop";
  return testThread->message_loop();
}
void sendMessage()
{
  std::string str("{ \"type\": \"EVENT_DLNA_DMSLIST_CHANGE\" }");
  s_dlna_event_handler(enum_DlnaEvent(0), 0, (char*)str.c_str());
}
}

namespace dlna {

static int s_dlna_log_printf( char* str, int len)
{
	LOG(INFO) << str;
  return 0;
}

//static
void DLNAService::instantiate() {
  // We need the AtExitManager to be created at the very beginning.
  g_at_exit_manager = new base::AtExitManager();
  DLNAService* dlna_service = new DLNAService();
  dlna_service->initialized();
  LOG(INFO) << "DLNAService registered.";
}
void DLNAService::DMREventCallback(int eventID, const char* eventArg, std::string* out)
{
  std::cout << __FUNCTION__ << ": eventID=" << eventID << ",eventArg=" << (char*)eventArg << std::endl;
  LOG(INFO) << __FUNCTION__ << ": eventID=" << eventID << ",eventArg=" << (char*)eventArg;

  std::string str("{ \"Position\": \"0\", \"Duration\": \"0\", \"returncode\": \"0\", \"Track\": \"0\", \"RelCount\": \"0\", \"AbsCount\": \"0\", \"TrackMetaData\": \"\", \"TrackURI\": \"\" }");

  if (out)
   *out = str;
  return;
}

//-------------------------------------------------------------------------------
DLNAService::DLNAService()
{
  LOG(INFO) << __FUNCTION__ << " starting...";
  g_dlna_service = this;
  LOG(INFO) << __FUNCTION__ << " started.";
}
DLNAService::~DLNAService()
{
  LOG(INFO) << "DLNAService destroyed.";
  UpnpFinish();
}
bool DLNAService::initialized()
{
  int ret = -1;
  //Initialized DLNA here.
//  Dlna_ModeInit(enum_DlnaAppMode_HUAWEI_V1R5, s_dlna_callback,
//                s_dlna_event_handler, enum_DlnaEvent_JsonString);
  ret = UpnpInit(NULL, 0);
  if (ret == UPNP_E_SUCCESS) {
    ret = UpnpSetWebServerRootDir( g_web_dir_path );
    if( ret != UPNP_E_SUCCESS ) {
      LOG(FATAL) << __FUNCTION__ << ": error=" << ret;
      UpnpFinish();
    }
  } else {
    LOG(FATAL) << __FUNCTION__ << ": error=" << ret;
    UpnpFinish();
  }
  UpnpSetMaxContentLength(202*1024);
//  HT_Initialize(s_dlna_log_printf);

#ifndef NDEBUG

//  HT_EnableModule(HT_MOD_UPNP);
//  HT_EnableModule(HT_MOD_DMC);
//  HT_EnableModule(HT_MOD_DMS);
//  HT_EnableModule(HT_MOD_DMR);
//  HT_EnableModule(HT_MOD_APP);
// /HT_EnableModule(HT_MOD_HYSDK);
//  HT_EnableModule(HT_MOD_IPC);

//  HT_EnableLevel(HT_BIT_KEY);
//  HT_EnableLevel(HT_BIT_FEW);
//  HT_EnableLevel(HT_BIT_MANY);
//  HT_EnableLevel(HT_BIT_MYRIAD);
#endif
  //Dmc_Init(0);
  //Dmc_Start();

  Dmr_Init(0);
  Dmr_Start(DMREventCallback);
  DLOG(INFO) << __FUNCTION__;
  while( true )
    Sleep( 1000);
  return true;
}

}
#endif

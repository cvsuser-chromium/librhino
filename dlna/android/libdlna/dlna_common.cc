#define LOG_NDEBUG 0
#define LOG_TAG "DLNACommon"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"

#include "dlna_api.h"
#include "dlna_api_cpp.h"

#include "hitTime.h"
#include <upnp.h>
#include <upnpdebug.h>

#include "dmp_device.h"
#include "dmr_device_client.h"
#include "dlna_common.h"

namespace android {

static DLNACommon DlnaCommon;
static bool bDlnaInit = false;

const char* g_web_dir_path = "/data/app/com.hybroad.dlna/webroot";
//const char* g_web_dir_path = "/system/iptv/dlna";

static int s_dlna_callback(enum_DlnaCallback eType, int value, char *str, void *user)
{
  int x,y,w,h;
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
  android::Parcel obj;
  base::StringPiece jsonstr(str);

  //notify always right.
  obj.writeInt32(6554);
  obj.writeString16(android::String16(str));
  int32_t msg = android::EVENT_DLNA_DMP_EVENT_START;

  if (base::StringPiece::npos != jsonstr.find("EVENT_DLNA_DMSLIST_CHANGE")) {
    msg = android::EVENT_DLNA_DMSLIST_CHANGED;
  }else if (base::StringPiece::npos != jsonstr.find("EVENT_DLNA_FILELIST_CHANGE")){
    msg = android::EVENT_DLNA_FILELIST_CHANGE;
  }else if (base::StringPiece::npos != jsonstr.find("EVENT_DLNA_CHANNEL_RESOURCE_CHANGED")){
    msg = android::EVENT_DLNA_CHANNEL_RESOURCE_CHANGED;
  }
  DlnaCommon.notify(msg, 0, 0, &obj);
  return 0;
}

static void DMREventCallback(int eventID, const char* eventArg, std::string* out)
{
  LOG(INFO) << __FUNCTION__ << ": eventID=" << eventID << ",eventArg=" << (char*)eventArg;
  android::Parcel obj;
  android::Parcel reply;

  //notify always right.
  obj.setDataPosition(0);
  obj.writeInt32(6554);
  obj.writeString16(android::String16((char*)eventArg));
  int32_t msg = android::EVENT_DLNA_DMR_EVENT_START;
  DlnaCommon.notify_sync(msg, eventID, &obj, &reply);
  reply.setDataPosition(0);
  android::String8 result = reply.readString8();

  //crap ....
  if (out) {
    out->clear();
    out->append(result.string(), result.length());
  }

//  std::string str("{ \"Position\": \"0\", \"Duration\": \"0\", \"returncode\": \"0\", \"Track\": \"0\", \"RelCount\": \"0\", \"AbsCount\": \"0\", \"TrackMetaData\": \"xxx\", \"TrackURI\": \"ddd\" }");
//  memcpy(arg, str.c_str(), str.length());
  return;
}

DLNACommon::DLNACommon()
{
  
}

DLNACommon::~DLNACommon()
{
    
}

DLNACommon& DLNACommon::GetInstance()
{
    return DlnaCommon;
}
static int s_dlna_log_printf( char* str, int len)
{
	LOG(INFO) << str;
}

int DLNACommon::Init(std::string& rootPath, std::string& dmrName,std::string& mac_addr)
{
    int ret = -1;

    LOG(INFO) << __FUNCTION__ << ": bDlnaInit = " << bDlnaInit;
    if (bDlnaInit)
        return 0;

    //Initialized DLNA here.
    Dlna_ModeInit(enum_DlnaAppMode_HUAWEI_V1R5, s_dlna_callback,
                  s_dlna_event_handler, enum_DlnaEvent_JsonString);
#ifdef DEBUG        
    UpnpSetLogFileNames("/data/log/upnpErr.log", "/data/log/upnpInfo.log");
#endif
    ret = UpnpInit(NULL, 0);
    LOG(INFO) << __FUNCTION__ << ": UpnpInit return  " << ret;
    if (ret == UPNP_E_SUCCESS) {
        ret = UpnpSetWebServerRootDir( rootPath.c_str() );
        if( ret != UPNP_E_SUCCESS ) {
            LOG(FATAL) << __FUNCTION__ << ": error=" << ret;
            UpnpFinish();
        }
    } else if (ret == UPNP_E_INIT){
        LOG(WARNING) << __FUNCTION__ << ": warning=" << ret;
        bDlnaInit = true;
        return 0;
    } else {
        LOG(FATAL) << __FUNCTION__ << ": error=" << ret;
        UpnpFinish();
    }
    UpnpSetMaxContentLength(202*1024);
    HT_Initialize(s_dlna_log_printf);

#ifndef NDEBUG
    //HT_EnableModule(HT_MOD_UPNP);
    //HT_EnableModule(HT_MOD_DMC);
    //HT_EnableModule(HT_MOD_DMS);
    HT_EnableModule(HT_MOD_DMR);
    HT_EnableModule(HT_MOD_APP);
    HT_EnableModule(HT_MOD_HYSDK);
    HT_EnableModule(HT_MOD_IPC);

    HT_EnableLevel(HT_BIT_KEY);
    HT_EnableLevel(HT_BIT_FEW);
    HT_EnableLevel(HT_BIT_MANY);
    HT_EnableLevel(HT_BIT_MYRIAD);
#endif
    Dmc_Init(0);
    Dmc_Start();

    Raw_Dmr_Init((char *)rootPath.c_str(),(char *)dmrName.c_str(),(char *)mac_addr.c_str());
    Dmr_Start(DMREventCallback);
    LOG(INFO) << __FUNCTION__;
      
    bDlnaInit = true;
    return 0;
}

int DLNACommon::Uninit()
{
    return 0;
}

int DLNACommon::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    int i,n, s32ret = 0;

    LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
    switch (msg){
    case EVENT_DLNA_DMP_EVENT_START:
    case EVENT_DLNA_DMSLIST_CHANGED:
    case EVENT_DLNA_FILELIST_CHANGE:
    case EVENT_DLNA_CHANNEL_RESOURCE_CHANGED:
    case EVENT_DLNA_DMP_EVENT_END:
        {
            base::AutoLock lock(mLock);
            for (i = 0, n = mDMPDevices.size(); i < n; ++i) {
                LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
                mDMPDevices[i]->notify(msg, ext1, ext2, obj);
            }
        }
        break;
    case EVENT_DLNA_DMR_EVENT_START:
    case EVENT_DLNA_DMR_EVENT_END:
        {
            LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
            base::AutoLock lock(mLock);
            for (i = 0, n = mDMRClients.size(); i < n; ++i) {
                LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
                s32ret |= mDMRClients[i]->notify(msg, ext1, ext2, obj);
            }
        }
        break;
    default:
        LOG(WARNING) << __FUNCTION__ << "type=["<< msg << "].";
        break;
    }
    return s32ret;
}

int DLNACommon::notify_sync(int msg, int eventID, const Parcel* obj, Parcel* reply )
{
    int i,n, s32ret = 0;

    LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
    switch (msg){
    case EVENT_DLNA_DMR_EVENT_START:
    case EVENT_DLNA_DMR_EVENT_END:
        {
            LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
            base::AutoLock lock(mLock);
            for (i = 0, n = mDMRClients.size(); i < n; ++i) {
                LOG(INFO) << __FUNCTION__ << "type=["<< msg << "].";
                s32ret |= mDMRClients[i]->notify_sync(msg, eventID, obj, reply);
            }
        }
        break;
    default:
        break;
    }
    return s32ret;
}

int DLNACommon::AddDMPDevice(sp<DMPDevice> device)
{
    base::AutoLock lock(mLock);
    mDMPDevices.add(device);
    return 0;
}

int DLNACommon::RemoveDMPDevice(sp<DMPDevice> device)
{
    base::AutoLock lock(mLock);
    mDMPDevices.remove(device);
    return 0;
}

int DLNACommon::AddDMRDeviceClient(sp<DMRDeviceClient> client)
{
    base::AutoLock lock(mLock);
    mDMRClients.add(client);
    return 0;    
}

int DLNACommon::RemoveDMRDeviceClient(sp<DMRDeviceClient> client)
{
    base::AutoLock lock(mLock);
    mDMRClients.remove(client);
    return 0;    
}

}

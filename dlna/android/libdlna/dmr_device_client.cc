#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_number_conversions.h"

#include "dlna_api.h"
#include "dlna_api_cpp.h"

#include "hitTime.h"
#include <upnp.h>

#include "dmp_device.h"
#include "dmr_device_client.h"
#include "dlna_common.h"

#include <string>
namespace android {
DMRDeviceClient::DMRDeviceClient()
{
    LOG(INFO) << "DMRDeviceClient construction.";
}

DMRDeviceClient::~DMRDeviceClient()
{
    LOG(INFO) << "DMRDeviceClient::~DMRDeviceClient: this=" << this;
    disconnect();
}

status_t DMRDeviceClient::notify_sync(int msg, int eventID, const Parcel* obj, Parcel* reply )
{
    base::AutoLock _l(mLock);
    LOG(INFO) << __FUNCTION__ << " msg=" << msg << " eventID=" << eventID;
    mListener->notify_sync(msg, eventID, obj, reply);
    return NO_ERROR;
}

bool DMRDeviceClient::init(std::string& rootPath, std::string dmrName,std::string mac_addr)
{
    DLNACommon& dlnaCommon = DLNACommon::GetInstance();

    LOG(INFO) << "init";

    dlnaCommon.Init(rootPath, dmrName, mac_addr);
    dlnaCommon.AddDMRDeviceClient(this);

    return true;
}

void DMRDeviceClient::died()
{
    base::AutoLock _l(mLock);
    LOG(INFO) << __FUNCTION__;
    notify_sync(EVENT_DLNA_ERROR, DLNA_ERROR_SERVER_DIED, 0, 0);
}

void DMRDeviceClient::disconnect()
{
    LOG(INFO) << __FUNCTION__;
}

status_t DMRDeviceClient::setListener(const sp<DMRDeviceClientListener>& listener)
{
    base::AutoLock _l(mLock);
    LOG(INFO) << "setListener";
    mListener = listener;
    return NO_ERROR;
}

status_t DMRDeviceClient::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    base::AutoLock _l(mLock);
    LOG(INFO) << __FUNCTION__ << " msg=" << msg << " ext1=" << ext1 <<
                    " ext2=" << ext2;
    if (mListener !=0 )
        mListener->notify(msg, ext1, ext2, obj);
    return 0;
}

status_t DMRDeviceClient::invoke(const Parcel& request, Parcel *reply)
{
    LOG(INFO) << __FUNCTION__;
    return 0;
}

status_t DMRDeviceClient::setParameter(int key, const Parcel& request)
{
    LOG(INFO) << __FUNCTION__;
    return 0;
}

status_t DMRDeviceClient::getParameter(int key, Parcel* reply)
{
    LOG(INFO) << __FUNCTION__;
    return 0;
}

status_t DMRDeviceClient::setResult(String8 result)
{
    base::AutoLock _l(mLock);
    LOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}
}

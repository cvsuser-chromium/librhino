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
DMPDevice::DMPDevice()
{
    LOG(INFO) << "DMPDevice construction.";
}

DMPDevice::~DMPDevice()
{
    LOG(INFO) << "DMPDevice::~DMPDevice";
    disconnect();
}

bool DMPDevice::init(std::string& rootPath, std::string dmrName,std::string mac_addr)
{
    DLNACommon& dlnaCommon = DLNACommon::GetInstance();

    LOG(INFO) << "init";
    dlnaCommon.Init(rootPath,dmrName,mac_addr);
    dlnaCommon.AddDMPDevice(this);
    return true;
}

void DMPDevice::disconnect()
{
    DLNACommon& dlnaCommon = DLNACommon::GetInstance();
    
    LOG(INFO) << __FUNCTION__;

    dlnaCommon.RemoveDMPDevice(this);
    return ;    
}

status_t DMPDevice::setListener(const sp<DMPDeviceListener>& listener)
{
    LOG(INFO) << "setListener";
    base::AutoLock _l(mLock);
    mListener = listener;
    return NO_ERROR;
}

void DMPDevice::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
    base::AutoLock _l(mLock);
    LOG(INFO) << __FUNCTION__ << " msg=" << msg << " ext1=" << ext1 <<
        " ext2=" << ext2;
    mListener->notify(msg, ext1, ext2, obj);
    return ;
}

status_t DMPDevice::invoke(const Parcel& request, Parcel *reply)
{
    LOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::setParameter(int key, const Parcel& request)
{
    LOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::getParameter(int key, Parcel* reply)
{
    LOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::setFriendlyName(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    LOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::researchDms(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    request.setDataPosition(0);
    int remove_all = request.readInt32();
    ::Dmc_SearchDms(remove_all);
    DLOG(INFO) << __FUNCTION__ << " remove_all: " << remove_all;
    reply->setDataPosition(0);
    reply->writeInt32(0);
    return NO_ERROR;
}

status_t DMPDevice::getDmsCount(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    char value[4096] = {0};
    std::string param("invalid");
    ::Dmc_HuaweiJse_V1R5("dlna.getDmsCount", param.c_str(), value, 4095, 1 );
    DLOG(INFO) << __FUNCTION__ << " return: " << value;

    int number0 = 0;
    base::StringToInt(std::string(value), &number0);
    reply->setDataPosition(0);
    reply->writeInt32(0);
    //DLOG(INFO) << __FUNCTION__ << " number0: " << number0;
    reply->writeInt32(number0);

    return NO_ERROR;
}

status_t DMPDevice::getDmsList(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    char value[4096] = {0};
    request.setDataPosition(0);
    int32 index = request.readInt32();
    int32 count = request.readInt32();
    //DLOG(INFO) << __FUNCTION__ << " index " << index << ", count " << count << ", request size " << request.dataSize();
    std::string param = base::StringPrintf("{\"index\":%d,\"count\":%d}",
                                         index, count);
    //DLOG(INFO) << __FUNCTION__ << " send param string: " << param.c_str();
    ::Dmc_HuaweiJse_V1R5("dlna.getDmsList", param.c_str(), value, 4095, 1 );
    DLOG(INFO) << __FUNCTION__ << " return: " << value;
    reply->setDataPosition(0);
    reply->writeInt32(0);
    reply->writeString16(String16(value));
    return NO_ERROR;
}

status_t DMPDevice::openFileListByContainer(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    char value[4096] = {0};
    request.setDataPosition(0);
    String8 deviceID(request.readString16());
    String8 containerID(request.readString16());
    //String8 classID(request.readString16());
    int sort = request.readInt32();
    int order = request.readInt32();
    std::string param = base::StringPrintf("{\"deviceID\":\"%s\",\"containerID\":\"%s\",\"sort\":%d,\"order\":\"%d\"}",
                        deviceID.string(), containerID.string(), sort,order );
    //DLOG(INFO) << __FUNCTION__ << " send param string: " << param.c_str();
    ::Dmc_HuaweiJse_V1R5("dlna.openFileList", param.c_str(), value, 4095, 1 );
    DLOG(INFO) << __FUNCTION__ << " return: " << value;
    reply->setDataPosition(0);
    reply->writeInt32(0);
    reply->writeString16(String16(value));
    return NO_ERROR;
}

status_t DMPDevice::getCount(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    char value[4096] = {0};
    request.setDataPosition(0);
    int32 listID = request.readInt32();
    std::string param = base::StringPrintf("{\"listID\":%d}", listID);
    //DLOG(INFO) << __FUNCTION__ << " send param string: " << param.c_str();
    ::Dmc_HuaweiJse_V1R5("dlna.getCount", param.c_str(), value, 4095, 1 );
    DLOG(INFO) << __FUNCTION__ << " return: " << value;

    int number0 = 0;
    base::StringToInt(std::string(value), &number0);
    reply->setDataPosition(0);
    reply->writeInt32(0);
    reply->writeInt32(number0);
    return NO_ERROR;
}

status_t DMPDevice::getList(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    char value[4096] = {0};
    request.setDataPosition(0);
    int32 listID = request.readInt32();
    int32 index = request.readInt32();
    int32 count = request.readInt32();
    std::string param = base::StringPrintf("{\"listID\":%d,\"index\":%d,\"count\":%d}",
                                             listID, index, count);
    //DLOG(INFO) << __FUNCTION__ << " send param string: " << param.c_str();
    ::Dmc_HuaweiJse_V1R5("dlna.getList", param.c_str(), value, 4095, 1 );
    DLOG(INFO) << __FUNCTION__ << " return: " << value;
    reply->setDataPosition(0);
    reply->writeInt32(0);
    reply->writeString16(String16(value));
    return NO_ERROR;
}

status_t DMPDevice::openFileListByClassID(const Parcel& request, Parcel* reply)
{
    base::AutoLock _l(mLock);

    char value[4096] = {0};
    request.setDataPosition(0);
    String8 deviceID(request.readString16());
    String8 classID(request.readString16());
    int sort = request.readInt32();
    String8 order(request.readString16());
    std::string param = base::StringPrintf("{\"deviceID\":\"%s\",\"classID\":\"%s\",\"sort\":%d,\"order\":\"%s\"}",
                            deviceID.string(), classID.string(), sort,order.string() );
    //DLOG(INFO) << __FUNCTION__ << " send param string: " << param.c_str();
    ::Dmc_HuaweiJse_V1R5("dlna.openFileList", param.c_str(), value, 4095, 1 );
    DLOG(INFO) << __FUNCTION__ << " return: " << value;
    reply->setDataPosition(0);
    reply->writeInt32(0);
    reply->writeString16(String16(value));
    return NO_ERROR;
}

status_t DMPDevice::getGroupInfo(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::getGroupLlist(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::getGroupItemList(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::openFileList_tunerChannel(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::localDB_deleteByDMS(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::localDB_searchByKeyWord(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::localDB_getSearchList(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::programItem_count_schedule(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::getEpg(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::bookmark_delete(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

status_t DMPDevice::dmrPlayer_stateSync(const Parcel& request, Parcel* reply)
{
    DLOG(INFO) << __FUNCTION__;
    return NO_ERROR;
}

}

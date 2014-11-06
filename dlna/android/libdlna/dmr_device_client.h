#ifndef ANDROID_DLNA_LIBDLNA_DMR_DEVICE_H
#define ANDROID_DLNA_LIBDLNA_DMR_DEVICE_H

#include "rhino_export.h"
#include "base/synchronization/lock.h"

#include <utils/Log.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <binder/Parcel.h>

namespace android {

// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class DMRDeviceClientListener : public RefBase
{
public:
    virtual status_t notify(int msg, int ext1, int ext2, const Parcel *obj) = 0;
    virtual status_t notify_sync(int msg, int eventID, const Parcel *obj, Parcel* reply) = 0;
};

class RHINO_EXPORT DMRDeviceClient : public RefBase
{
public:
  DMRDeviceClient();
  ~DMRDeviceClient();

  status_t notify(int msg, int ext1, int ext2, const Parcel *obj);
  status_t notify_sync(int msg, int eventID, const Parcel* obj, Parcel* reply );

  bool            init(std::string& rootPath, std::string dmrName, std::string mac_addr);
  void            died();
  void            disconnect();
  status_t        setListener(const sp<DMRDeviceClientListener>& listener);
  status_t        invoke(const Parcel& request, Parcel *reply);
  status_t        setParameter(int key, const Parcel& request);
  status_t        getParameter(int key, Parcel* reply);
  status_t        setResult(String8 result);

private:
  mutable  base::Lock                  mLock;
  sp<DMRDeviceClientListener>          mListener;
};

}; // namespace android

#endif // ANDROID_MEDIAPLAYER_H

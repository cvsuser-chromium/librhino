#ifndef DLNA_COMMON_H
#define DLNA_COMMON_H

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

enum dlna_event_type {
  EVENT_DLNA_NOP               = 0, // interface test message
  EVENT_DLNA_DMP_EVENT_START   = 1,
  EVENT_DLNA_DMSLIST_CHANGED   = 2,
  EVENT_DLNA_FILELIST_CHANGE   = 3,
  EVENT_DLNA_CHANNEL_RESOURCE_CHANGED = 4,
  EVENT_DLNA_DMP_EVENT_END     = 99,

  EVENT_DLNA_DMR_EVENT_START   = 100,
  EVENT_DLNA_DMR_EVENT_END     = 199,

  EVENT_DLNA_ERROR             = 400,
  EVENT_DLNA_INFO              = 500,
};
// Generic error codes for the media player framework.  Errors are fatal, the
// playback must abort.
//
// Errors are communicated back to the client using the
// MediaPlayerListener::notify method defined below.
// In this situation, 'notify' is invoked with the following:
//   'msg' is set to MEDIA_ERROR.
//   'ext1' should be a value from the enum media_error_type.
//   'ext2' contains an implementation dependant error code to provide
//          more details. Should default to 0 when not used.
//
// The codes are distributed as follow:
//   0xx: Reserved
//   1xx: Android Player errors. Something went wrong inside the MediaPlayer.
//   2xx: Media errors (e.g Codec not supported). There is a problem with the
//        media itself.
//   3xx: Runtime errors. Some extraordinary condition arose making the playback
//        impossible.
//
enum dlna_error_type {
  // 0xx
  DLNA_ERROR_UNKNOWN = 1,
  // 1xx
  DLNA_ERROR_SERVER_DIED = 100,
  // 2xx
  DLNA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200,
  // 3xx
};

class DLNACommon 
{
    public:    
        static DLNACommon& GetInstance();

        DLNACommon();
        ~DLNACommon();

        int Init(std::string& rootPath, std::string& dmrName, std::string& mac_addr);
        int Uninit();

        int AddDMPDevice(sp<DMPDevice> device);
        int RemoveDMPDevice(sp<DMPDevice> device);

        int AddDMRDeviceClient(sp<DMRDeviceClient> client);
        int RemoveDMRDeviceClient(sp<DMRDeviceClient> client);

        int notify(int msg, int ext1, int ext2, const Parcel *obj);
        int notify_sync(int msg, int eventID, const Parcel* obj, Parcel* reply );

    private:
        mutable  base::Lock mLock;
        SortedVector< sp<DMPDevice> > mDMPDevices;
        SortedVector< sp<DMRDeviceClient> > mDMRClients;
};

}
#endif

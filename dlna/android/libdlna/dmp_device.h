#ifndef ANDROID_DLNA_LIBDLNA_DMP_DEVICE_H
#define ANDROID_DLNA_LIBDLNA_DMP_DEVICE_H

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

enum media_event_type {
    MEDIA_NOP               = 0, // interface test message
    MEDIA_PREPARED          = 1,
    MEDIA_PLAYBACK_COMPLETE = 2,
    MEDIA_BUFFERING_UPDATE  = 3,
    MEDIA_SEEK_COMPLETE     = 4,
    MEDIA_SET_VIDEO_SIZE    = 5,
    //z00180556 add begin
    MEDIA_FAST_FORWORD_COMPLETE     = 20,
    MEDIA_FAST_BACKWORD_COMPLETE    = 21,
    //z00180556 add end
    MEDIA_TIMED_TEXT        = 99,
    MEDIA_ERROR             = 100,
    MEDIA_INFO              = 200,
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
enum media_error_type {
    // 0xx
    MEDIA_ERROR_UNKNOWN = 1,
    // 1xx
    MEDIA_ERROR_SERVER_DIED = 100,
    // 2xx
    MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200,
    // 3xx
};


// Info and warning codes for the media player framework.  These are non fatal,
// the playback is going on but there might be some user visible issues.
//
// Info and warning messages are communicated back to the client using the
// MediaPlayerListener::notify method defined below.  In this situation,
// 'notify' is invoked with the following:
//   'msg' is set to MEDIA_INFO.
//   'ext1' should be a value from the enum media_info_type.
//   'ext2' contains an implementation dependant info code to provide
//          more details. Should default to 0 when not used.
//
// The codes are distributed as follow:
//   0xx: Reserved
//   7xx: Android Player info/warning (e.g player lagging behind.)
//   8xx: Media info/warning (e.g media badly interleaved.)
//
enum media_info_type {
    // 0xx
    MEDIA_INFO_UNKNOWN = 1,
    // 7xx
    // The video is too complex for the decoder: it can't decode frames fast
    // enough. Possibly only the audio plays fine at this stage.
    MEDIA_INFO_VIDEO_TRACK_LAGGING = 700,
    // MediaPlayer is temporarily pausing playback internally in order to
    // buffer more data.
    MEDIA_INFO_BUFFERING_START = 701,
    // MediaPlayer is resuming playback after filling buffers.
    MEDIA_INFO_BUFFERING_END = 702,
    // Bandwidth in recent past
    MEDIA_INFO_NETWORK_BANDWIDTH = 703,
    // The steps before player start
    MEDIA_INFO_PREPARE_PROGRESS = 710,


    // 8xx
    // Bad interleaving means that a media has been improperly interleaved or not
    // interleaved at all, e.g has all the video samples first then all the audio
    // ones. Video is playing but a lot of disk seek may be happening.
    MEDIA_INFO_BAD_INTERLEAVING = 800,
    // The media is not seekable (e.g live stream).
    MEDIA_INFO_NOT_SEEKABLE = 801,
    // New media metadata is available.
    MEDIA_INFO_METADATA_UPDATE = 802,
    //z00180556 add begin
    MEDIA_INFO_AUDIO_FAIL   = 1000, /*Audio play fail*/
    MEDIA_INFO_VIDEO_FAIL   = 1001, /*Video play fail*/
    MEDIA_INFO_NETWORK      = 1002, /*network erro/unknown*/
    MEDIA_INFO_TIMEOUT      = 1003, /*time out*/
    MEDIA_INFO_NOT_SUPPORT  = 1004, /*media not support*/
    MEDIA_INFO_BUFFER_EMPTY = 1005, /*net-player buffer is empty*/
    MEDIA_INFO_BUFFER_START = 1006, /*net-player buffer is starting*/
    MEDIA_INFO_BUFFER_ENOUGH= 1007, /*net-player buffer is enough*/
    MEDIA_INFO_BUFFER_FULL  = 1008, /*net-player buffer is full*/
    MEDIA_INFO_BUFFER_DOWNLOAD_FIN = 1009, /*net-player buffer download finish*/
    MEDIA_INFO_FIRST_FRAME_TIME = 1010, /* The Fist frame time */
    MEDIA_INFO_STREM_3D_MODE = 1011, /* stream 3D mode */
    MEDIA_INFO_STREM_IFRAME_ERROR  = 1012, /* I frame error */
    MEDIA_INFO_STREM_NORM_SWITCH   = 1013, /* stream norm switch */
    //z00180556 add end
};

// Keep KEY_PARAMETER_* in sync with MediaPlayer.java.
// The same enum space is used for both set and get, in case there are future keys that
// can be both set and get.  But as of now, all parameters are either set only or get only.
enum media_parameter_keys {
    KEY_PARAMETER_TIMED_TEXT_TRACK_INDEX = 1000,                // set only
    KEY_PARAMETER_TIMED_TEXT_ADD_OUT_OF_BAND_SOURCE = 1001,     // set only

    // Streaming/buffering parameters
    KEY_PARAMETER_CACHE_STAT_COLLECT_FREQ_MS = 1100,            // set only

    // Return a Parcel containing a single int, which is the channel count of the
    // audio track, or zero for error (e.g. no audio track) or unknown.
    KEY_PARAMETER_AUDIO_CHANNEL_COUNT = 1200,                   // get only

};

// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class DMPDeviceListener : public RefBase
{
public:
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj) = 0;
};

class RHINO_EXPORT DMPDevice : public RefBase
{
public:
    DMPDevice();
    ~DMPDevice();
    bool            init(std::string& rootPath, std::string dmrName, std::string mac_addr);
    void            disconnect();
    
    status_t        setListener(const sp<DMPDeviceListener>& listener);
    void            notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);
    status_t        invoke(const Parcel& request, Parcel *reply);
    status_t        setParameter(int key, const Parcel& request);
    status_t        getParameter(int key, Parcel* reply);

    status_t        setFriendlyName(const Parcel& request, Parcel* reply);
    status_t        researchDms(const Parcel& request, Parcel* reply);
    status_t        getDmsCount(const Parcel& request, Parcel* reply);
    status_t        getDmsList(const Parcel& request, Parcel* reply);
    status_t        openFileListByContainer(const Parcel& request, Parcel* reply);
    status_t        getCount(const Parcel& request, Parcel* reply);
    status_t        getList(const Parcel& request, Parcel* reply);
    status_t        openFileListByClassID(const Parcel& request, Parcel* reply);
    status_t        getGroupInfo(const Parcel& request, Parcel* reply);
    status_t        getGroupLlist(const Parcel& request, Parcel* reply);
    status_t        getGroupItemList (const Parcel& request, Parcel* reply);
    status_t        openFileList_tunerChannel(const Parcel& request, Parcel* reply);
    status_t        localDB_deleteByDMS(const Parcel& request, Parcel* reply);
    status_t        localDB_searchByKeyWord(const Parcel& request, Parcel* reply);
    status_t        localDB_getSearchList(const Parcel& request, Parcel* reply);
    status_t        programItem_count_schedule(const Parcel& request, Parcel* reply);
    status_t        getEpg(const Parcel& request, Parcel* reply);
    status_t        bookmark_delete(const Parcel& request, Parcel* reply);
    status_t        dmrPlayer_stateSync(const Parcel& request, Parcel* reply);

private:
    mutable  base::Lock        mLock;
    sp<DMPDeviceListener>      mListener;
};

}; // namespace android

#endif // ANDROID_MEDIAPLAYER_H

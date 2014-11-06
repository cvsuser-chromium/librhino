#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "base/lazy_instance.h"

#include <android_os_Parcel.h>

#include "libdlna/dmp_device.h"
#include "jni/DMPDevice_jni.h"
#include "rhino_jni_registrar.h"
#include <string>
// ----------------------------------------------------------------------------

using namespace android;

// ----------------------------------------------------------------------------
namespace {
struct fields_t {
    jfieldID    context;
    jmethodID   post_event;
};
static fields_t fields;

static base::LazyInstance<base::Lock> sLock;
}
// ----------------------------------------------------------------------------
// ref-counted object for callbacks
class JNIDMPDeviceListener: public DMPDeviceListener
{
public:
    JNIDMPDeviceListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNIDMPDeviceListener();
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);
private:
    JNIDMPDeviceListener();
    base::android::ScopedJavaGlobalRef<jclass>      mClass;     // Reference to DMPDevice class
    base::android::ScopedJavaGlobalRef<jobject>     mObject;    // Weak ref to DMPDevice Java object to call on
};

JNIDMPDeviceListener::JNIDMPDeviceListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{
    // Hold onto the MediaPlayer class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        LOG(ERROR) << "Can't find android/media/MediaPlayer";
        return;
    }
    mClass.Reset(env, clazz);

    // We use a weak reference so the DMPDevice object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    mObject.Reset(env, weak_thiz);
}

JNIDMPDeviceListener::~JNIDMPDeviceListener()
{
  LOG(INFO) << "JNIDMPDeviceListener::~JNIDMPDeviceListener ";
}

void JNIDMPDeviceListener::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  {
  base::android::ScopedJavaLocalRef<jstring> jString;
  if (obj && obj->dataSize() > 0) {
    //reset read position.
    obj->setDataPosition(0);

    //ignore status code.
    status_t status = obj->readInt32();

    LOG(INFO) << "JNIDMPDeviceListener::notify: " << status;
    android::String16 json_str = obj->readString16();
    jString.Reset(env, env->NewString(json_str.string(), json_str.size()));
    Java_DMPDevice_postEventFromNative(env, mObject.obj(), msg, ext1, ext2, jString.obj());
  }else {
    LOG(INFO) << "JNIDMPDeviceListener::notify: null string!";
    Java_DMPDevice_postEventFromNative(env, mObject.obj(), msg, ext1, ext2, NULL);
  }
  }
  base::android::DetachFromVM();
}
// ----------------------------------------------------------------------------
static sp<DMPDevice> getDMPDevice(JNIEnv* env, jobject thiz)
{
    base::AutoLock l(sLock.Get());
    DMPDevice* const p = (DMPDevice*)env->GetIntField(thiz, fields.context);
    return sp<DMPDevice>(p);
}

static sp<DMPDevice> setDMPDevice(JNIEnv* env, jobject thiz, const sp<DMPDevice>& player)
{
    base::AutoLock l(sLock.Get());
    sp<DMPDevice> old = (DMPDevice*)env->GetIntField(thiz, fields.context);
    if (player.get()) {
        player->incStrong(thiz);
    }
    if (old != 0) {
        old->decStrong(thiz);
    }
    env->SetIntField(thiz, fields.context, (int)player.get());
    return old;
}
// ----------------------------------------------------------------------------


static void _setLooping(JNIEnv* env, jobject obj, jboolean looping)
{
}

static jboolean _isLooping(JNIEnv* env, jobject obj)
{
  return false;
}

static jboolean _setParameter(JNIEnv *env, jobject obj, jint key, jobject value)
{
    LOG(INFO) << "setParameter: key  " << key;
    sp<DMPDevice> mp = getDMPDevice(env, obj);
    if (mp == NULL ) {
        return false;
    }

    Parcel *request = parcelForJavaObject(env, value);
    status_t err = mp->setParameter(key, *request);
    if (err == OK) {
        return true;
    } else {
        return false;
    }
}
static void _getParameter(JNIEnv* env, jobject thiz, jint key, jobject java_reply)
{
    LOG(INFO) <<"getParameter: key " << key;
    sp<DMPDevice> mp = getDMPDevice(env, thiz);
    if (mp == NULL ) {
        return;
    }

    Parcel *reply = parcelForJavaObject(env, java_reply);
    //process_media_player_call(env, thiz, mp->getParameter(key, reply), NULL, NULL );
}

static jint _invoke(JNIEnv* env, jobject obj,
    jobject java_request,
    jobject java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->invoke(*request, reply);
}

//called from java side.
// This function gets some field IDs, which in turn causes class initialization.
// It is called from a static block in DMPDevice, which won't run until the
// first time an instance of this class is used.
static void _init(JNIEnv* env, jclass clazz)
{
  fields.context = env->GetFieldID(clazz, "mNativeDMPDevice", "I");
  if (fields.context == NULL) {
      return;
  }
}

/*
BASE_EXPORT void ConvertJavaStringToUTF8(JNIEnv* env,
                                         jstring str,
                                         std::string* result);
                                         */
static void _setup(JNIEnv* env, jobject obj,
    jobject DMPDevice_this,
    jstring root_path,
    jstring dmrName){
  sp<DMPDevice> mp = new DMPDevice();
  LOG(INFO) << "native_setup";
  if (mp == NULL) {
      return;
  }
  std::string rootDir;
  std::string deviceName;

  base::android::ConvertJavaStringToUTF8(env, root_path, &rootDir);
  base::android::ConvertJavaStringToUTF8(env, dmrName, &deviceName);
  mp->init(rootDir,deviceName);
  // create new listener and give it to MediaPlayer
  sp<JNIDMPDeviceListener> listener = new JNIDMPDeviceListener(env, obj, DMPDevice_this);
  mp->setListener(listener);

  // Stow our new C++ MediaPlayer in an opaque field in the Java object.
  setDMPDevice(env, obj, mp);
}
static void _release(JNIEnv* env, jobject thiz)
{
  LOG(INFO) <<"release";
  sp<DMPDevice> mp = setDMPDevice(env, thiz, 0);
  if (mp != NULL) {
    // this prevents native callbacks after the object is released
    mp->setListener(0);
    mp->disconnect();
  }
}

static void _finalize(JNIEnv *env, jobject thiz)
{
  LOG(INFO) <<"native_finalize";
  sp<DMPDevice> mp = getDMPDevice(env, thiz);
  DLOG_IF(WARNING, mp != NULL) << "DMPDevice finalized without being released";
  _release(env, thiz);
}
static jint _researchDms(JNIEnv* env, jobject obj,
                         jobject java_request,
                         jobject java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }
  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);
  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->researchDms(*request, reply);
}

static jint _getDmsCount(JNIEnv* env, jobject obj,
                         jobject java_request,
                         jobject java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }
  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);
  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getDmsCount(*request, reply);
}
static jint _getCount(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getCount(*request, reply);
}

static jint _getDmsList(JNIEnv* env, jobject obj,
    jobject java_request,
    jobject java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getDmsList(*request, reply);
}

static jint _openFileListByContainer(JNIEnv* env, jobject obj,
                                     jobject  java_request,
                                     jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->openFileListByContainer(*request, reply);
}
static jint _getList(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getList(*request, reply);
}
static jint _openFileListByClassID(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->openFileListByClassID(*request, reply);
}
static jint _getGroupInfo(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getGroupInfo(*request, reply);
}
static jint _getGroupLlist(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getGroupLlist(*request, reply);
}
static jint _getGroupItemList(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getGroupItemList(*request, reply);
}
static jint _openFileList_tunerChannel(JNIEnv* env, jobject obj,
    jobject  java_request,
    jobject  java_reply)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->openFileList_tunerChannel(*request, reply);
}

static jint _localDB_deleteByDMS(JNIEnv* env, jobject obj, jstring deviceID)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel request;
  Parcel reply;

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->localDB_deleteByDMS(request, &reply);
}
static jint _localDB_searchByKeyWord(JNIEnv* env, jobject obj,
    jstring keyWord,
    jstring matchRule)
{
  sp<DMPDevice> dmp_device = getDMPDevice(env, obj);
  if (dmp_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel request;
  Parcel reply;

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmp_device->getList(request, &reply);
}
static jint _localDB_getSearchList(JNIEnv* env, jobject obj,
    jint listID,
    jint index,
    jint count,
    jint classID)
{
}

bool RegisterDlnaDMPDevice(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

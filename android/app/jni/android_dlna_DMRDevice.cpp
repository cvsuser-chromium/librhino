#include "base/android/jni_android.h"
#include "base/android/jni_string.h"

#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "base/lazy_instance.h"

#include <android_os_Parcel.h>

#include "libdlna/dmr_device_client.h"
#include "jni/DMRDeviceClient_jni.h"
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
class JNIDMRDeviceClientListener: public DMRDeviceClientListener
{
public:
    JNIDMRDeviceClientListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNIDMRDeviceClientListener();
    virtual status_t notify(int msg, int ext1, int ext2, const Parcel *obj = NULL);
    virtual status_t notify_sync(int msg, int eventID, const Parcel *obj, Parcel* reply);
private:
    JNIDMRDeviceClientListener();
    base::android::ScopedJavaGlobalRef<jclass>      mClass;     // Reference to DMPDevice class
    base::android::ScopedJavaGlobalRef<jobject>     mObject;    // Weak ref to DMPDevice Java object to call on
};

JNIDMRDeviceClientListener::JNIDMRDeviceClientListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{
    // Hold onto the MediaPlayer class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        LOG(ERROR) << "Can't find com/hybroad/DMRDeviceClient";
        return;
    }
    mClass.Reset(env, clazz);

    // We use a weak reference so the DMPDevice object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
    mObject.Reset(env, weak_thiz);
}

JNIDMRDeviceClientListener::~JNIDMRDeviceClientListener()
{
  LOG(INFO) << "JNIDMRDeviceClientListener::~JNIDMRDeviceClientListener";
}

status_t JNIDMRDeviceClientListener::notify(int msg, int ext1, int ext2, const Parcel *obj)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  {
	  base::android::ScopedJavaLocalRef<jstring> jString;
	  if (obj && obj->dataSize() > 0) {
	    //reset read position.
	    obj->setDataPosition(0);

	    //ignore status code.
	    status_t status = obj->readInt32();

	    LOG(INFO) << "JNIDMRDeviceClientListener::notify: " << status;
	    android::String16 json_str = obj->readString16();
	    jString.Reset(env, env->NewString(json_str.string(), json_str.size()));
	    Java_DMRDeviceClient_postEventFromNative(env, mObject.obj(), msg, ext1, ext2, jString.obj());
	  }else {
	    LOG(INFO) << "JNIDMRDeviceClientListener::notify: null string!";
	    Java_DMRDeviceClient_postEventFromNative(env, mObject.obj(), msg, ext1, ext2, NULL);
	  }
    }
    base::android::DetachFromVM();
  return 0;
}

static base::subtle::AtomicWord g_String_getBytes = 0;
void jstringTostring(JNIEnv* env, jstring jstr, String8* result)
{
  base::android::ScopedJavaLocalRef<jclass> clsstring(env, env->FindClass("java/lang/String"));
  base::android::ScopedJavaLocalRef<jstring> strencode(env, env->NewStringUTF("utf-8"));
//  jmethodID mid = env->GetMethodID(clsstring.obj(), "getBytes", "(Ljava/lang/String;)[B");
  jmethodID mid =
     base::android::MethodID::LazyGet<
     base::android::MethodID::TYPE_INSTANCE>(
      env, clsstring.obj(),
      "getBytes","(Ljava/lang/String;)[B",
      &g_String_getBytes);

  base::android::ScopedJavaLocalRef<jbyteArray> barr(env,
        (jbyteArray)env->CallObjectMethod(jstr, mid, strencode.obj()));
  jsize alen = env->GetArrayLength(barr.obj());
  jbyte* ba = env->GetByteArrayElements(barr.obj(), JNI_FALSE);
  if (alen > 0 && result) {
    result->append((const char*)ba, alen);
  }
  env->ReleaseByteArrayElements(barr.obj(), ba, 0);
  base::android::CheckException(env);
  return ;
}

status_t JNIDMRDeviceClientListener::notify_sync(int msg, int eventID, const Parcel *obj, Parcel* reply)
{
  JNIEnv *env = base::android::AttachCurrentThread();
    {
	 base::android::ScopedJavaLocalRef<jstring> jString;
	  ScopedJavaLocalRef<jstring> jResult;

	  if (obj && obj->dataSize() > 0) {
	    //reset read position.
	    obj->setDataPosition(0);
	    //ignore status code.
	    status_t status = obj->readInt32();
	    android::String16 json_str = obj->readString16();
	    jString.Reset(env, env->NewString(json_str.string(), json_str.size()));
	    LOG(INFO) << "JNIDMRDeviceClientListener::notify_sync: " << status
	          << ", json_str: " << json_str;
	    jResult = Java_DMRDeviceClient_postEventFromNativeSync(env, mObject.obj(), msg, eventID, 0, jString.obj());
	  }else {
	    LOG(INFO) << "JNIDMRDeviceClientListener::notify: null string!";
	    jResult = Java_DMRDeviceClient_postEventFromNativeSync(env, mObject.obj(), msg, eventID, 0, NULL);
	  }
	  if (reply) {
	    android::String8 result;
	    jstringTostring(env, jResult.obj(), &result);
	    reply->writeString8(result);
	  }
    }
    base::android::DetachFromVM();
  return 0;
}

// ----------------------------------------------------------------------------
static sp<DMRDeviceClient> getDMRDeviceClient(JNIEnv* env, jobject thiz)
{
    base::AutoLock l(sLock.Get());
    DMRDeviceClient* const p = (DMRDeviceClient*)env->GetIntField(thiz, fields.context);
    return sp<DMRDeviceClient>(p);
}

static sp<DMRDeviceClient> setDMRDeviceClient(JNIEnv* env, jobject thiz, const sp<DMRDeviceClient>& player)
{
    base::AutoLock l(sLock.Get());
    sp<DMRDeviceClient> old = (DMRDeviceClient*)env->GetIntField(thiz, fields.context);
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

static jboolean _setParameter(JNIEnv *env, jobject obj, jint key, jobject value)
{
    LOG(INFO) << "setParameter: key  " << key;
    sp<DMRDeviceClient> mp = getDMRDeviceClient(env, obj);
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
    sp<DMRDeviceClient> mp = getDMRDeviceClient(env, thiz);
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
  sp<DMRDeviceClient> dmr_device = getDMRDeviceClient(env, obj);
  if (dmr_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *request = parcelForJavaObject(env, java_request);
  Parcel *reply = parcelForJavaObject(env, java_reply);

  // Don't use process_media_player_call which use the async loop to
  // report errors, instead returns the status.
  return dmr_device->invoke(*request, reply);
}

//called from java side.
// This function gets some field IDs, which in turn causes class initialization.
// It is called from a static block in DMPDevice, which won't run until the
// first time an instance of this class is used.
static void _init(JNIEnv* env, jclass clazz)
{
  fields.context = env->GetFieldID(clazz, "mNativeDMRDeviceClient", "I");
  if (fields.context == NULL) {
      return;
  }
}

static void _setup(JNIEnv* env, jobject obj,
    jobject DMRDevice_this,
    jstring root_path,
    jstring dmrName){
  sp<DMRDeviceClient> mp = new DMRDeviceClient();
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
  sp<JNIDMRDeviceClientListener> listener = new JNIDMRDeviceClientListener(env, obj, DMRDevice_this);
  mp->setListener(listener);

  // Stow our new C++ MediaPlayer in an opaque field in the Java object.
  setDMRDeviceClient(env, obj, mp);
}
static void _release(JNIEnv* env, jobject thiz)
{
  LOG(INFO) <<"release";
  sp<DMRDeviceClient> mp = setDMRDeviceClient(env, thiz, 0);
  if (mp != NULL) {
    // this prevents native callbacks after the object is released
    mp->setListener(0);
    mp->disconnect();
  }
}

static void _finalize(JNIEnv *env, jobject thiz)
{
  LOG(INFO) <<"native_finalize";
  sp<DMRDeviceClient> mp = getDMRDeviceClient(env, thiz);
  DLOG_IF(WARNING, mp != NULL) << "DMPDevice finalized without being released";
  _release(env, thiz);
}
static jint _setResult(JNIEnv* env, jobject obj, jobject java_result)
{
  sp<DMRDeviceClient> dmr_device = getDMRDeviceClient(env, obj);
  if (dmr_device == NULL ) {
      return UNKNOWN_ERROR;
  }

  Parcel *result = parcelForJavaObject(env, java_result);

  String8 order(result->readString16());

  return dmr_device->setResult(order);
}

bool RegisterDlnaDMRDeviceClient(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

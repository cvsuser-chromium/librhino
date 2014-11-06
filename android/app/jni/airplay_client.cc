#include "base/android/jni_android.h"
#include "base/android/jni_string.h"
#include "base/android/jni_array.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/lazy_instance.h"
#include "base/values.h"
#include "base/synchronization/lock.h"

#include "network/NetworkServices.h"
#include "interfaces/AnnouncementManager.h"

#include <string>
#include "airplay_client.h"
#include "jni/AirPlayClient_jni.h"
#include "rhino_jni_registrar.h"


namespace rhino{
static jint Init(JNIEnv* env, jclass clazz,
    jobject AirPlayClient_this)
{
  AirPlayClient* airplay_client =
    new AirPlayClient(env, clazz, AirPlayClient_this);

  return reinterpret_cast<jint>(airplay_client);
}
AirPlayClient::AirPlayClient(JNIEnv* env, jobject thiz, jobject weak_thiz)
{
  // Hold onto the MediaPlayer class for use in calling the static method
  // that posts events to the application thread.
  LOG(INFO) << "AirPlayClient::AirPlayClient";
  jclass clazz = env->GetObjectClass(thiz);
  if (clazz == NULL) {
    LOG(ERROR) << "Can't find com/hybroad/AirPlayClient";
    //base::android::jniThrowException(env, "java/lang/Exception", NULL);
    return;
  }
  mClass.Reset(env, clazz);

  // We use a weak reference so the DMPDevice object can be garbage collected.
  // The reference is only used as a proxy for callbacks.
  mObject.Reset(env, weak_thiz);
  Initialize();
}

AirPlayClient::~AirPlayClient()
{
  LOG(INFO) << "AirPlayClient::~AirPlayClient";
}
void AirPlayClient::Initialize() {
  CAirPlayServer::Get()->SetDelegate(this);
  CAirTunesServer::Get()->SetDelegate(this);
  CNetworkServices::Get().Start();
}
void AirPlayClient::Destroy(JNIEnv* env, jobject obj) {
  CAirPlayServer::Get()->SetDelegate(0);
  CAirTunesServer::Get()->SetDelegate(0);
  delete this;
}
int AirPlayClient::AnnounceToClients(JNIEnv* env, jobject obj, jstring jsonStr)
{
  std::string json_message;
  std::string sender;
  std::string message;

  base::android::ConvertJavaStringToUTF8(env, jsonStr, &json_message);
  int error_code;

  base::Value* value = base::JSONReader::ReadAndReturnError(json_message, 0, &error_code, &message);

  if (!value){
    LOG(WARNING) << "Got Wrong message format" << json_message;
    return -1;
  }
  if (!value->IsType(base::Value::TYPE_DICTIONARY)) {
    //raise a exception to Java side.
  }
  base::DictionaryValue *message_dict = reinterpret_cast<base::DictionaryValue*>(value);
  message_dict->GetString("sender", &sender);
  message_dict->GetString("status", &message);
  ANNOUNCEMENT::CAnnouncementManager::Announce(ANNOUNCEMENT::Player, sender.c_str(), message.c_str(), json_message.c_str());
  return 0;
}

int AirPlayClient::onEvent(CAirPlayServer::Delegate::EVENTID eventID, const std::string& eventArg)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jString =
    base::android::ConvertUTF8ToJavaString( env, eventArg);

  LOG(INFO) << "AirPlayClient::onEvent.";
  Java_AirPlayClient_postEventFromNative(env, mObject.obj(), eventID, jString.obj());
  return 0;
}

int AirPlayClient::onEventSync(CAirPlayServer::Delegate::EVENTID eventID, const std::string& eventArg, std::string* result)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jString =
        base::android::ConvertUTF8ToJavaString( env, eventArg);
  ScopedJavaLocalRef<jstring> jResult;

  LOG(INFO) << "AirPlayClient::onEventSync.";

  jResult = Java_AirPlayClient_postEventFromNativeSync(env, mObject.obj(), eventID, jString.obj());

  if (result) {;
    base::android::ConvertJavaStringToUTF8(env, jResult.obj(), result);
  }
  return 0;
}

int AirPlayClient::PublishAirPlayService(
          const std::string& fcr_identifier,
          const std::string& fcr_type,
          const std::string& fcr_name,
          int f_port,
          std::vector<std::pair<std::string, std::string> > txt)
{
  LOG(INFO) << "PublishAirPlayService.";
  return registerNsdServices(fcr_identifier, fcr_type, fcr_name, f_port, txt);
}
int AirPlayClient::PublishAirTunesService(
          const std::string& fcr_identifier,
          const std::string& fcr_type,
          const std::string& fcr_name,
          int f_port,
          std::vector<std::pair<std::string, std::string> > txt)
{
  LOG(INFO) << "PublishAirTunesService.";
  return registerNsdServices(fcr_identifier, fcr_type, fcr_name, f_port, txt);
}

void* AirPlayClient::audio_init(int bits, int channels, int samplerateHZ)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  ScopedJavaLocalRef<jstring> jResult;
  LOG(INFO) << "AirPlayClient::audio_init.";

  jResult = Java_AirPlayClient_audioInit(env, mObject.obj(), bits, channels, samplerateHZ);
  return 0;
}
void AirPlayClient::audio_set_volume(const char* session, int volume)
{
  LOG(INFO) << "session=" << (char*)session << " volume=" << volume;
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jSession =
              base::android::ConvertUTF8ToJavaString( env, session);

  Java_AirPlayClient_audioSetVolumeByNative(env, mObject.obj(), jSession.obj(), volume);
}
void AirPlayClient::audio_set_metadata(const char* session, const std::string& key, std::string value)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jSession =
                base::android::ConvertUTF8ToJavaString( env, session);
  base::android::ScopedJavaLocalRef<jstring> jStringKey =
                base::android::ConvertUTF8ToJavaString( env, key);
  base::android::ScopedJavaLocalRef<jstring> jStringValue =
                base::android::ConvertUTF8ToJavaString( env, value);
  LOG(INFO) << "audio_set_metadata";
  Java_AirPlayClient_audioSetMetadata(env, mObject.obj(), jSession.obj(), jStringKey.obj(), jStringValue.obj());
}
void AirPlayClient::audio_set_coverart(const char* session, const void *buffer, int buflen)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  //Java_AirPlayClient_audioSetCoverart();
  LOG(INFO) << "audio_set_coverart";
}
void AirPlayClient::audio_process(const char* session, const uint8 *buffer, int buflen)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jSession =
              base::android::ConvertUTF8ToJavaString( env, session);
  base::android::ScopedJavaLocalRef<jbyteArray> byte_array =
              base::android::ToJavaByteArray(env, buffer, buflen);
  Java_AirPlayClient_audioDataProcess(env, mObject.obj(), jSession.obj(), byte_array.obj(), buflen);
 // LOG(INFO) << "audio_process.";
}
void AirPlayClient::audio_flush(const char* session)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jSession =
                base::android::ConvertUTF8ToJavaString( env, session);
  Java_AirPlayClient_audioFlush(env, mObject.obj(), jSession.obj());
  LOG(INFO) << "audio_flush.";
}
void AirPlayClient::audio_destroy(const char* session)
{
  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jSession =
                base::android::ConvertUTF8ToJavaString( env, session);
  Java_AirPlayClient_audioDestroy(env, mObject.obj(), jSession.obj());

  jSession.Reset();
  //i'll goto die. so detacch from dalvik.
  base::android::DetachFromVM();
  LOG(INFO) << "audio_destroy.";
}

int AirPlayClient::registerNsdServices(
  const std::string& fcr_identifier,
  const std::string& fcr_type,
  const std::string& fcr_name,
  int f_port,
  std::vector<std::pair<std::string, std::string> > txt)
{
  int ret=-1;
  DictionaryValue service_dict;
  service_dict.Set("type", new StringValue(fcr_type));
  service_dict.Set("name", new StringValue(fcr_name));
  service_dict.Set("port", new base::FundamentalValue(f_port));
  {
    DictionaryValue* txt_dict = new DictionaryValue();
    for(size_t i=0; i < txt.size(); i++) {
      txt_dict->Set(txt[i].first, new StringValue(txt[i].second));
    }      service_dict.Set("txtRecord", txt_dict);
  }
  std::string eventArg;
  base::JSONWriter::Write(&service_dict, &eventArg);

  JNIEnv *env = base::android::AttachCurrentThread();
  base::android::ScopedJavaLocalRef<jstring> jString =
    base::android::ConvertUTF8ToJavaString(env, eventArg);
  ret = Java_AirPlayClient_registerNsdServices(env, mObject.obj(), jString.obj());
  //current version we always publish service in both android and rhino side.
  //so tell client we are failed.
  LOG(INFO) << "publish service command: " << eventArg << "result=" << ret;
  return ret;
}


}
// ----------------------------------------------------------------------------
bool RegisterAirPlayClient(JNIEnv* env)
{
  return rhino::RegisterNativesImpl(env);
}


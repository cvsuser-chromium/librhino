#ifndef ANDROID_APP_JNI_AIRPLAY_CLIENT_H_
#define ANDROID_APP_JNI_AIRPLAY_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/synchronization/lock.h"
#include "network/airplay_server.h"
#include "network/AirTunesServer.h"

namespace rhino{
// ----------------------------------------------------------------------------
class AirPlayClient : public CAirPlayServer::Delegate
                     ,public CAirTunesServer::Delegate
{
public:
  AirPlayClient(JNIEnv* env, jobject thiz, jobject weak_thiz);
  ~AirPlayClient();
  // --------------------------------------------------------------------------
  // Methods called from Java via JNI
  // --------------------------------------------------------------------------
  static jint Init(JNIEnv* env, jclass clazz);
  void Destroy(JNIEnv* env, jobject obj);
  int AnnounceToClients(JNIEnv* env, jobject obj, jstring jsonStr);
  void Initialize();

  virtual int onEvent(EVENTID eventID, const std::string& eventArg) OVERRIDE;
  virtual int onEventSync(EVENTID eventID, const std::string& eventArg, std::string* out) OVERRIDE;
  virtual int PublishAirPlayService(
          const std::string& fcr_identifier,
          const std::string& fcr_type,
          const std::string& fcr_name,
          int f_port,
          std::vector<std::pair<std::string, std::string> > txt) OVERRIDE;
  virtual int PublishAirTunesService(
          const std::string& fcr_identifier,
          const std::string& fcr_type,
          const std::string& fcr_name,
          int f_port,
          std::vector<std::pair<std::string, std::string> > txt) OVERRIDE;

  virtual void* audio_init(int bits, int channels, int samplerateHZ) OVERRIDE;
  virtual void  audio_set_volume(const char* session, int volume) OVERRIDE;
  virtual void  audio_set_metadata(const char* session, const std::string& key, std::string value) OVERRIDE;
  virtual void  audio_set_coverart(const char* session, const void *buffer, int buflen) OVERRIDE;
  virtual void  audio_process(const char* session, const uint8 *buffer, int buflen) OVERRIDE;
  virtual void  audio_flush(const char* session) OVERRIDE;
  virtual void  audio_destroy(const char* session) OVERRIDE;

private:
  AirPlayClient();
  int registerNsdServices(
      const std::string& fcr_identifier,
      const std::string& fcr_type,
      const std::string& fcr_name,
      int f_port,
      std::vector<std::pair<std::string, std::string> > txt);
  base::android::ScopedJavaGlobalRef<jclass>      mClass;     // Reference to DMPDevice class
  base::android::ScopedJavaGlobalRef<jobject>     mObject;    // Weak ref to DMPDevice Java object to call on
};
}
#endif

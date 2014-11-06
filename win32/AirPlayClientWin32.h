#ifndef WIN32_AIRPLAYCLIENT_WIN32_H_
#define WIN32_AIRPLAYCLIENT_WIN32_H_

#include "base/compiler_specific.h"
#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"

#include "network/airplay_server.h"
#include "network/AirTunesServer.h"

namespace rhino{
// ----------------------------------------------------------------------------
class AirPlayClient
  : public CAirPlayServer::Delegate
  , public CAirTunesServer::Delegate
  , public base::RefCountedThreadSafe<AirPlayClient>
{
public:
  AirPlayClient();
  ~AirPlayClient();

  void Initialize();
  virtual int onEvent(EVENTID eventID, const std::string& eventArg) OVERRIDE;
  virtual int onEventSync(EVENTID eventID, const std::string& eventArg, std::string* out) OVERRIDE;
  virtual int PublishAirPlayService(const std::string& fcr_identifier,
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
  virtual void  audio_set_coverart(const char* session, const void *buffer, int buflen ) OVERRIDE;
  virtual void  audio_process(const char* session, const uint8 *buffer, int buflen) OVERRIDE;
  virtual void  audio_flush(const char* session) OVERRIDE;
  virtual void  audio_destroy(const char* session) OVERRIDE;
private:
  int registerNsdServices(
      const std::string& fcr_identifier,
      const std::string& fcr_type,
      const std::string& fcr_name,
      int f_port,
      std::vector<std::pair<std::string, std::string> > txt);
  void sendFeedback(CAirPlayServer::Delegate::EVENTID eventId, const char* mesage);
  base::WeakPtrFactory<AirPlayClient> weak_factory_;
};
}
#endif

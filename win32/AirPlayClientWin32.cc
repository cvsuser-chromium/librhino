#include "base/bind.h"
#include "base/logging.h"
#include "base/synchronization/lock.h"
#include "base/message_loop/message_loop.h"
#include "base/lazy_instance.h"

#include <string>
#include "AirPlayClientWin32.h"
#include "interfaces/AnnouncementManager.h"
#include "network/NetworkServices.h"
namespace rhino{
AirPlayClient::AirPlayClient( )
  :weak_factory_(this)
{
  // Hold onto the MediaPlayer class for use in calling the static method
  // that posts events to the application thread.
  LOG(INFO) << "AirPlayClient::AirPlayClient";
  Initialize();
}

AirPlayClient::~AirPlayClient()
{
  CAirPlayServer::Get()->SetDelegate(0);
  CAirTunesServer::Get()->SetDelegate(0);
  LOG(INFO) << "AirPlayClient::~AirPlayClient";
}
void AirPlayClient::Initialize() {
  CNetworkServices::Get().Start();
  CAirPlayServer::Get()->SetDelegate(this);
  CAirTunesServer::Get()->SetDelegate(this);
}
void AirPlayClient::sendFeedback(CAirPlayServer::Delegate::EVENTID eventId, const char* msg)
{
  //base::DictionaryValue *message_dict = reinterpret_cast<base::DictionaryValue*>(value);
  //message_dict->GetString("sender", &sender);
  //message_dict->GetString("status", &message);
  std::string sender = "xbmc";
  std::string message = msg;
  //ANNOUNCEMENT::CAnnouncementManager::Announce(ANNOUNCEMENT::Player, sender.c_str(), message.c_str());
}
int AirPlayClient::onEvent(CAirPlayServer::Delegate::EVENTID eventId, const std::string& eventArg)
{
  LOG(INFO) << "AirPlayClient::onEvent. " << eventArg;
  std::string result;
  onEventSync(eventId, eventArg, &result);
  return 0;
}

int AirPlayClient::onEventSync(CAirPlayServer::Delegate::EVENTID eventId, const std::string& eventArg, std::string* result)
{
  LOG(INFO) << "AirPlayClient::onEventSync.";

  switch (eventId) {
  case EVENT_DMR_PLAY:

    base::MessageLoop::current()->PostDelayedTask(FROM_HERE,
      base::Bind(&AirPlayClient::sendFeedback, weak_factory_.GetWeakPtr(), eventId, "OnPlay"),
      base::TimeDelta::FromMilliseconds(800));
    break;
  case EVENT_DMR_GETPOSITIONINFO:
    if (result)
    result->assign("{ \"Position\": 2, \"Duration\": 6297, \"returncode\": \"0\", \"Track\": \"0\", \
      \"RelCount\": \"0\", \"AbsCount\": \"0\", \"TrackURI\": \"http://192.168.12.126:7001/1/1ed1f827-d8f6-52e3-bc51-3e36629398f0.mp4\" }");
    break;
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
  return registerNsdServices(fcr_identifier, fcr_type, fcr_name, f_port, txt);
}
int AirPlayClient::PublishAirTunesService( 
      const std::string& fcr_identifier,
      const std::string& fcr_type,
      const std::string& fcr_name,
      int f_port,
      std::vector<std::pair<std::string, std::string> > txt)
{
  return registerNsdServices(fcr_identifier, fcr_type, fcr_name, f_port, txt);
}

void* AirPlayClient::audio_init(int bits, int channels, int samplerateHZ)
{
  return 0;
}
void AirPlayClient::audio_set_volume(const char* session, int volume)
{
}
void AirPlayClient::audio_set_metadata(const char* session, const std::string& key, std::string value)
{
}
void AirPlayClient::audio_set_coverart(const char* session, const void *buffer, int buflen)
{
}
void AirPlayClient::audio_process(const char* session, const uint8 *buffer, int buflen)
{
}
void AirPlayClient::audio_flush(const char* session)
{
}
void AirPlayClient::audio_destroy(const char* session)
{
}
int AirPlayClient::registerNsdServices(
      const std::string& fcr_identifier,
      const std::string& fcr_type,
      const std::string& fcr_name,
      int f_port,
      std::vector<std::pair<std::string, std::string> > txt)
{
  return -1;
}
}


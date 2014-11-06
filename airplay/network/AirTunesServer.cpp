/*
 * Many concepts and protocol specification in this code are taken
 * from Shairport, by James Laird.
 *
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include "base/strings/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/lazy_instance.h"
#include "network/Network.h"
#if !defined(TARGET_WINDOWS)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "AirTunesServer.h"
#include "shairplay/raop.h"

#ifdef HAS_AIRPLAY
#include "network/airplay_server.h"
#endif

#ifdef HAS_AIRTUNES

#include "utils/log.h"
#include "network/Zeroconf.h"
#include "utils/Variant.h"
#include "settings/AdvancedSettings.h"
#include "utils/EndianSwap.h"
#include "interfaces/AnnouncementManager.h"
#include "Application.h"

#include <map>
#include <string>

using namespace ANNOUNCEMENT;
base::LazyInstance<CAirTunesServer> gServerInstance = LAZY_INSTANCE_INITIALIZER;
std::string CAirTunesServer::m_macAddress;
namespace {
const char* kSessionId = "hybroad-airtunes";
}
//parse daap metadata - thx to project MythTV
CAirTunesServer::Metadata decodeDMAP(const char *buffer, unsigned int size)
{
  std::map<std::string, std::string> result;
  unsigned int offset = 8;
  while (offset < size)
  {
    std::string tag;
    tag.append(buffer + offset, 4);
    offset += 4;
    uint32_t length = Endian_SwapBE32(*(uint32_t *)(buffer + offset));
    offset += sizeof(uint32_t);
    std::string content;
    content.append(buffer + offset, length);//possible fixme - utf8?
    offset += length;
    result[tag] = content;
  }
  return result;
}

void CAirTunesServer::SetMetadataFromBuffer(void *session, const char *buffer, unsigned int size)
{
  Metadata metadata = decodeDMAP(buffer, size);
  std::map<std::string, std::string>::const_iterator it = metadata.begin();
  while (it != metadata.end()) {
    LOG(INFO)<< "key=" << it->first << " value=" << it->second;
    if (gServerInstance.Get().m_delegate)
      gServerInstance.Get().m_delegate->audio_set_metadata(kSessionId, it->first, it->second);
    it ++;
  }
}

void CAirTunesServer::Announce(AnnouncementFlag flag, const char *sender, const char *message, const std::string& json_data)
{
  if ( (flag & Player) && strcmp(sender, "xbmc") == 0 && strcmp(message, "OnStop") == 0)
  {
#ifdef HAS_AIRPLAY
    CAirPlayServer::restoreVolume();
#endif
  }
}
CAirTunesServer::Delegate* CAirTunesServer::SetDelegate( CAirTunesServer::Delegate* delegate)
{
  Delegate* old = m_delegate;
  m_delegate = delegate;
  return old;
}
void CAirTunesServer::SetCoverArtFromBuffer(void *session, const char *buffer, unsigned int size)
{
  if (gServerInstance.Get().m_delegate)
    gServerInstance.Get().m_delegate->audio_set_coverart(kSessionId, buffer, size);
  else
    LOG(ERROR_REPORT) << "null delegate.";
}

#define RSA_KEY " \
-----BEGIN RSA PRIVATE KEY-----\
MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\
wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\
wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\
/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\
UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\
BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\
LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\
NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\
lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\
aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\
a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\
oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\
oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\
k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\
AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\
cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\
54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\
17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\
1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\
LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\
2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\
-----END RSA PRIVATE KEY-----"

void CAirTunesServer::AudioOutputFunctions::audio_set_metadata(void *cls, void *session, const void *buffer, int buflen)
{
  CAirTunesServer::SetMetadataFromBuffer(session, (char *)buffer, buflen);
}

void CAirTunesServer::AudioOutputFunctions::audio_set_coverart(void *cls, void *session, const void *buffer, int buflen)
{
  CAirTunesServer::SetCoverArtFromBuffer(session, (char *)buffer, buflen);
}

void* CAirTunesServer::AudioOutputFunctions::audio_init(void *cls, int bits, int channels, int samplerateHZ)
{
  LOG(INFO) << "bits=" << bits << " channels=" << channels << " samplerate=" << samplerateHZ;
  CAirTunesServer* server = static_cast<CAirTunesServer*>(cls);
  if (server){
    return server->audio_init(bits, channels, samplerateHZ);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
  return 0;
}

void  CAirTunesServer::AudioOutputFunctions::audio_set_volume(void *cls, void *session, float volume)
{
  //volume from -30 - 0 - -144 means mute
  float volPercent = volume < -30.0f ? 0 : 1 - volume/-30;
  LOG(INFO) << "volume: " << volPercent;
  CAirTunesServer* server = static_cast<CAirTunesServer*>(cls);
  if (server) {
    server->audio_set_volume(session, (volPercent*100));
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
}

int  CAirTunesServer::AudioOutputFunctions::audio_process(void *cls, void *session, const void *buffer, int buflen)
{
  // LOG(INFO) << "Got data lengh: " << buflen;
  CAirTunesServer* server = static_cast<CAirTunesServer*>(cls);
  if (server) {
    return server->audio_process((char*)session, (const uint8*)buffer, buflen);
  }

  LOG(ERROR_REPORT) << "null delegate.";
  return 0;
}

void  CAirTunesServer::AudioOutputFunctions::audio_flush(void *cls, void *session)
{
 CAirTunesServer* server = static_cast<CAirTunesServer*>(cls);
  if (server) {
    server->audio_flush((char*)session);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
}

void  CAirTunesServer::AudioOutputFunctions::audio_destroy(void *cls, void *session)
{
  LOG(INFO) << "run here.";
  CAirTunesServer* server = static_cast<CAirTunesServer*>(cls);
  if (server){
    server->audio_destroy(session);
  } else {
    LOG(ERROR_REPORT) << "null server.";
  }
}
void* CAirTunesServer::audio_init(int bits, int channels, int samplerateHZ)
{
  static int client_id_counter = 0;
  if (m_delegate) {
    m_delegate->audio_init(bits, channels, samplerateHZ);
    mSession = ++ client_id_counter;
    session_id_ = base::IntToString(mSession);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
  return (void*)mSession;
}
void CAirTunesServer::audio_set_volume(void *session, float volume)
{
  int tmp_session_id = (int)session;
  std::string session_id = base::IntToString(tmp_session_id);
  if (m_delegate && mSession == tmp_session_id) {
    m_delegate->audio_set_volume(kSessionId, volume);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
}
void CAirTunesServer::audio_set_metadata(void *session, const void *buffer, int buflen)
{
  SetMetadataFromBuffer(session, (char *)buffer, buflen);
}
void CAirTunesServer::audio_set_coverart(void *session, const void *buffer, int buflen)
{
  SetCoverArtFromBuffer(session, (char *)buffer, buflen);
}
int CAirTunesServer::audio_process(void *session, const void *buffer, int buflen)
{
  int session_id = (int)session;
  if (m_delegate && mSession == session_id) {
    m_delegate->audio_process(kSessionId, (const uint8*)buffer, buflen);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
    return -1;
    //return -2;
  }
  return 0;
}
void CAirTunesServer::audio_flush(void *session)
{
  int session_id = (int)session;
  if (m_delegate && mSession == session_id) {
    m_delegate->audio_flush(kSessionId);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
}
void CAirTunesServer::audio_destroy(void *session)
{
  int session_id = (int)session;
  if (m_delegate && mSession == session_id) {
    m_delegate->audio_destroy(kSessionId);
  } else {
    LOG(ERROR_REPORT) << "null delegate.";
  }
}
void shairplay_log(void *cls, int level, const char *msg)
{
  int xbmcLevel = LOGINFO;

  switch (level) {
  case RAOP_LOG_EMERG:    // system is unusable
    xbmcLevel = LOGFATAL;
    break;
  case RAOP_LOG_ALERT:    // action must be taken immediately
  case RAOP_LOG_CRIT:     // critical conditions
    xbmcLevel = LOGSEVERE;
    break;
  case RAOP_LOG_ERR:      // error conditions
    xbmcLevel = LOGERROR;
    break;
  case RAOP_LOG_WARNING:  // warning conditions
    xbmcLevel = LOGWARNING;
    break;
  case RAOP_LOG_NOTICE:   // normal but significant condition
    xbmcLevel = LOGNOTICE;
    break;
  case RAOP_LOG_INFO:     // informational
    xbmcLevel = LOGINFO;
    break;
  case RAOP_LOG_DEBUG:    // debug-level messages
    xbmcLevel = LOGDEBUG;
    break;
  default:
    break;
  }
  LOG(INFO) << "AIRTUNES: %s" << msg;
}

bool CAirTunesServer::StartServer(int port, bool nonlocal, bool usePassword, const std::string& password/*=""*/)
{
  bool success = false;
  std::string pw = password;
  CNetworkInterface *net = CApplication::getInstance().getNetwork().GetFirstConnectedInterface();
  StopServer(true);

  if (net) {
    std::string mac(net->GetMacAddress());
    RemoveChars(mac, ":", &m_macAddress);
    while (m_macAddress.size() < 12) {
      m_macAddress = std::string("0") + m_macAddress;
    }
  } else {
    m_macAddress = "000102030405";
  }

  if (!usePassword) {
    pw.clear();
  }

  if (gServerInstance.Get().Initialize(pw,port, nonlocal)) {
    success = true;
    gServerInstance.Get().PublishService();
  }

  return success;
}

void CAirTunesServer::StopServer(bool bWait)
{
    gServerInstance.Get().StopThread(bWait);
    gServerInstance.Get().Deinitialize();

    CZeroconf::GetInstance()->RemoveService("servers.airtunes");
}

bool CAirTunesServer::IsRunning()
{
  return ((CThread*)gServerInstance.Pointer())->IsRunning();
}

CAirTunesServer::CAirTunesServer() : CThread("AirTunesServer")
  ,m_delegate(0)
{
  //m_port = port;
  m_pRaop = 0;
  CAnnouncementManager::AddAnnouncer(this);
}

CAirTunesServer::~CAirTunesServer()
{
  CAnnouncementManager::RemoveAnnouncer(this);
  m_delegate = 0;
}
CAirTunesServer* CAirTunesServer::Get()
{
  return gServerInstance.Pointer();
}
void CAirTunesServer::Process()
{
  m_bStop = false;
}

bool CAirTunesServer::Initialize(const std::string &password, int port, bool nonlocal)
{
  int ret = false;
  int raop_error = -1;
  Deinitialize();
  raop_callbacks_t ao;
  ao.cls                  = this;//m_pPipe;
  ao.audio_init           = AudioOutputFunctions::audio_init;
  ao.audio_set_volume     = AudioOutputFunctions::audio_set_volume;
  ao.audio_set_metadata   = AudioOutputFunctions::audio_set_metadata;
  ao.audio_set_coverart   = AudioOutputFunctions::audio_set_coverart;
  ao.audio_process        = AudioOutputFunctions::audio_process;
  ao.audio_flush          = AudioOutputFunctions::audio_flush;
  ao.audio_destroy        = AudioOutputFunctions::audio_destroy;

  m_passwd = password;
  m_port = port;

  //1 - we handle one client at a time max
  m_pRaop = raop_init(1, &ao, RSA_KEY, &raop_error);
  ret = (m_pRaop != NULL);

  if (ret) {
    char macAdr[6];
    unsigned short port = (unsigned short)m_port;

    raop_set_log_level(m_pRaop, RAOP_LOG_WARNING);
    if (CAdvancedSettings::getInstance().m_logEnableAirtunes) {
      raop_set_log_level(m_pRaop, RAOP_LOG_DEBUG);
    }

    raop_set_log_callback(m_pRaop, shairplay_log, NULL);

    CNetworkInterface *net = CApplication::getInstance().getNetwork().GetFirstConnectedInterface();

    if (net) {
      net->GetMacAddressRaw(macAdr);
    }

    ret = raop_start(m_pRaop, &port, macAdr, 6, password.c_str()) >= 0;
    if (ret < 0) {
      port = 0;
      ret = raop_start(m_pRaop, &port, macAdr, 6, password.c_str()) >= 0;
      if (ret >= 0)
        m_port = port;
    }
  }
  return ret;
}
bool CAirTunesServer::PublishService( )
{
  int ret = -1;
  const char* kFcr_identifier = "servers.airtunes";
  const char* kFr_type = "_raop._tcp";

  std::string appName(m_macAddress.c_str());
#if defined(WIN32)
  appName += "@hybroad-pc";
#else
  appName += "@hybroad-";
  appName += m_macAddress;
#endif

  std::vector<std::pair<std::string, std::string> > txt;
  txt.push_back(std::make_pair("txtvers",  "1"));
  txt.push_back(std::make_pair("cn", "0,1"));
  txt.push_back(std::make_pair("ch", "2"));
  txt.push_back(std::make_pair("ek", "1"));
  txt.push_back(std::make_pair("et", "0,1"));
  txt.push_back(std::make_pair("sv", "false"));
  txt.push_back(std::make_pair("tp",  "UDP"));
  txt.push_back(std::make_pair("sm",  "false"));
  txt.push_back(std::make_pair("ss",  "16"));
  txt.push_back(std::make_pair("sr",  "44100"));
  txt.push_back(std::make_pair("pw",  m_passwd.empty() ? "false" : "true"));
  txt.push_back(std::make_pair("vn",  "3"));
  txt.push_back(std::make_pair("da",  "true"));
  txt.push_back(std::make_pair("vs",  "130.14"));
  txt.push_back(std::make_pair("md",  "0,1,2"));
  txt.push_back(std::make_pair("am",  "Xbmc,1"));

  if (m_delegate) {
    LOG(INFO) << "PublishService Airplay";
    ret = m_delegate->PublishAirTunesService(kFcr_identifier, kFr_type, appName, m_port, txt);
  }
  if (ret < 0) {
    LOG(INFO) << "PublishService Airplay";
    CZeroconf::GetInstance()->PublishService(kFcr_identifier, kFr_type, appName, m_port, txt);
  }
  return true;
}
void CAirTunesServer::Deinitialize()
{
  if (m_pRaop) {
    raop_stop(m_pRaop);
    raop_destroy(m_pRaop);
    m_pRaop = 0;
  }
}

#endif


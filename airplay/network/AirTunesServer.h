#pragma once
/*
 * Many concepts and protocol specification in this code are taken from
 * the Boxee project. http://www.boxee.tv
 *
 *      Copyright (C) 2011-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
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
#include "system.h"
#include "base/basictypes.h"
#ifdef HAS_AIRTUNES

#include <string>
#include <vector>
#include <map>

#include "threads/Thread.h"
#include "threads/CriticalSection.h"
#include "utils/HttpParser.h"
#include "utils/StdString.h"
#include "interfaces/IAnnouncer.h"
//#include "shairplay/raop.h"

struct raop_t;
class CAirTunesServer : public CThread, public ANNOUNCEMENT::IAnnouncer
{
public:
  typedef std::map<std::string, std::string> Metadata;

  class Delegate {
  public:
    virtual int PublishAirTunesService(
      const std::string& fcr_identifier,
      const std::string& fcr_type,
      const std::string& fcr_name,
      int f_port,
      std::vector<std::pair<std::string, std::string> > txt) = 0;
    virtual void* audio_init(int bits, int channels, int samplerateHZ) = 0;
    virtual void  audio_set_volume(const char* session, int volume) = 0;
    virtual void  audio_set_metadata(const char* session, const std::string& key, std::string value) = 0;
    virtual void  audio_set_coverart(const char* session, const void *buffer, int buflen) = 0;
    virtual void  audio_process(const char* session, const uint8 *buffer, int buflen) = 0;
    virtual void  audio_flush(const char* session) = 0;
    virtual void  audio_destroy(const char* session) = 0;
  };
  virtual void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const std::string& json_data) OVERRIDE;

  Delegate* SetDelegate(Delegate*);

  static bool StartServer(int port, bool nonlocal, bool usePassword, const std::string &password="");
  static void StopServer(bool bWait);
  static bool IsRunning();
  static void SetMetadataFromBuffer(void *session, const char *buffer, unsigned int size);
  static void SetCoverArtFromBuffer(void *session, const char *buffer, unsigned int size);

  void* audio_init(int bits, int channels, int samplerateHZ);
  void  audio_set_volume(void *session, float volume);
  void  audio_set_metadata(void *session, const void *buffer, int buflen);
  void  audio_set_coverart(void *session, const void *buffer, int buflen);
  int  audio_process(void *session, const void *buffer, int buflen);
  void  audio_flush(void *session);
  void  audio_destroy(void *session);

  CAirTunesServer();
  ~CAirTunesServer();
  static CAirTunesServer* Get();
protected:
  void Process();

private:
  CAirTunesServer(int port, bool nonlocal);

  bool Initialize(const std::string &password,int port, bool nonlocal);
  bool PublishService();
  void Deinitialize();

  int m_port;
  raop_t *m_pRaop;
  Delegate* m_delegate;
  int mSession;
  //session for jni.
  std::string session_id_;

  std::string m_passwd;
  static std::string m_macAddress;

  class AudioOutputFunctions
  {
  public:
    static void* audio_init(void *cls, int bits, int channels, int samplerateHZ);
    static void  audio_set_volume(void *cls, void *session, float volume);
    static void  audio_set_metadata(void *cls, void *session, const void *buffer, int buflen);
    static void  audio_set_coverart(void *cls, void *session, const void *buffer, int buflen);
    static int  audio_process(void *cls, void *session, const void *buffer, int buflen);
    static void  audio_flush(void *cls, void *session);
    static void  audio_destroy(void *cls, void *session);
  };
};

#endif

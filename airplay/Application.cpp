#include "base/lazy_instance.h"

#include "network/Network.h"
#include "threads/SystemClock.h"
#include "system.h"
#include "Application.h"
#include "network/Zeroconf.h"
#include "network/ZeroconfBrowser.h"
#ifndef TARGET_POSIX
#include "threads/platform/win/Win32Exception.h"
#endif
#ifdef HAS_AIRPLAY
#include "network/airplay_server.h"
#endif
#ifdef HAS_AIRTUNES
#include "network/AirTunesServer.h"
#endif

namespace {
  base::LazyInstance<CApplication> gApplication;
}
CApplication& CApplication::getInstance()
{
  return gApplication.Get();
}
CApplication::CApplication(void)
{
  Initialize();
}

CApplication::~CApplication(void)
{
}

bool CApplication::Initialize()
{
#if defined(HAS_LINUX_NETWORK)
  m_network = new CNetworkLinux();
#elif defined(HAS_WIN32_NETWORK)
  m_network = new CNetworkWin32();
#else
  error;  m_network = new CNetwork();
#endif
  return true;
}

CNetwork& CApplication::getNetwork()
{
  return *m_network;
}
bool CApplication::StartServer(enum ESERVERS eServer, bool bStart, bool bWait/* = false*/)
{
  bool ret = false;
  //switch (eServer) {
  //case ES_AIRPLAYSERVER:
  //  // the callback will take care of starting/stopping airplay
  //  ret = CSettings::Get().SetBool("services.airplay", bStart);
  //  break;
  //case ES_UPNPSERVER:
  //  // the callback will take care of starting/stopping upnp server
  //  ret = CSettings::Get().SetBool("services.upnpserver", bStart);
  //  break;

  //case ES_UPNPRENDERER:
  //  // the callback will take care of starting/stopping upnp renderer
  //  ret = CSettings::Get().SetBool("services.upnprenderer", bStart);
  //  break;
  //case ES_ZEROCONF:
  //  // the callback will take care of starting/stopping zeroconf
  //  ret = CSettings::Get().SetBool("services.zeroconf", bStart);
  //  break;

  //default:
  //  ret = false;
  //  break;
  //}
  return ret;
}

bool CApplication::IsVideoScanning() const
{
  return false;
  //return m_videoInfoScanner->IsScanning();
}

bool CApplication::IsMusicScanning() const
{
  return false;
  //return m_musicInfoScanner->IsScanning();
}

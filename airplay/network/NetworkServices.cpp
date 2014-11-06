/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "NetworkServices.h"
#include "Application.h"
#include "network/Network.h"

#ifdef HAS_AIRPLAY
#include "network/airplay_server.h"
#endif // HAS_AIRPLAY

#ifdef HAS_AIRTUNES
#include "network/AirTunesServer.h"
#endif // HAS_AIRTUNES


#ifdef HAS_ZEROCONF
#include "network/Zeroconf.h"
#endif // HAS_ZEROCONF

#ifdef HAS_UPNP
#include "network/upnp/UPnP.h"
#endif // HAS_UPNP

#include "settings/AdvancedSettings.h"
#include "utils/log.h"


CNetworkServices::CNetworkServices()
{
}

CNetworkServices::~CNetworkServices()
{
}

CNetworkServices& CNetworkServices::Get()
{
  static CNetworkServices sNetworkServices;
  return sNetworkServices;
}

void CNetworkServices::Start()
{
  StartZeroconf();
  StartAirPlayServer();
  StartAirTunesServer();
}

void CNetworkServices::Stop(bool bWait)
{
  if (bWait) {
    StopUPnP(bWait);
    StopZeroconf();
  }

  StopAirPlayServer(bWait);
  StopAirTunesServer(bWait);
}


bool CNetworkServices::StartAirPlayServer()
{
#ifdef HAS_AIRPLAY
  if (IsAirPlayServerRunning())
    return true;
  if (!CAirPlayServer::StartServer(CAdvancedSettings::getInstance().m_airPlayPort, true))
    return false;

  //if (!CAirPlayServer::SetCredentials(CSettings::Get().GetBool("services.useairplaypassword"),
  //                                    CSettings::Get().GetString("services.airplaypassword")))
  //  return false;
  return true;
#endif // HAS_AIRPLAY
  return false;
}

bool CNetworkServices::IsAirPlayServerRunning()
{
#ifdef HAS_AIRPLAY
  return CAirPlayServer::isRunning();
#endif // HAS_AIRPLAY
  return false;
}

bool CNetworkServices::StopAirPlayServer(bool bWait)
{
#ifdef HAS_AIRPLAY
  if (!IsAirPlayServerRunning())
    return true;

  CAirPlayServer::StopServer(bWait);

#ifdef HAS_ZEROCONF
  CZeroconf::GetInstance()->RemoveService("servers.airplay");
#endif // HAS_ZEROCONF

  return true;
#endif // HAS_AIRPLAY
  return false;
}

bool CNetworkServices::StartAirTunesServer()
{
#ifdef HAS_AIRTUNES
  //if (!g_application.getNetwork().IsAvailable() || !CSettings::Get().GetBool("services.airplay"))
  //  return false;

  if (IsAirTunesServerRunning())
    return true;

  //if (!CAirTunesServer::StartServer(CAdvancedSettings::getInstance().m_airTunesPort, true,
  //                                  CSettings::Get().GetBool("services.useairplaypassword"),
  //                                  CSettings::Get().GetString("services.airplaypassword")))
  if (!CAirTunesServer::StartServer(CAdvancedSettings::getInstance().m_airTunesPort, true,
                                    false, "")) {
    LOG(ERROR_REPORT) << "Failed to start AirTunes Server";
    return false;
  }

  return true;
#endif // HAS_AIRTUNES
  return false;
}

bool CNetworkServices::IsAirTunesServerRunning()
{
#ifdef HAS_AIRTUNES
  return CAirTunesServer::IsRunning();
#endif // HAS_AIRTUNES
  return false;
}

bool CNetworkServices::StopAirTunesServer(bool bWait)
{
#ifdef HAS_AIRTUNES
  if (!IsAirTunesServerRunning())
    return true;

  CAirTunesServer::StopServer(bWait);
  return true;
#endif // HAS_AIRTUNES
  return false;
}

bool CNetworkServices::StartUPnP()
{
  bool ret = false;
#ifdef HAS_UPNP
  ret |= StartUPnPClient();
  ret |= StartUPnPServer();
  ret |= StartUPnPRenderer();
#endif // HAS_UPNP
  return ret;
}

bool CNetworkServices::StopUPnP(bool bWait)
{
#ifdef HAS_UPNP
  if (!CUPnP::IsInstantiated())
    return true;

  CLog::Log(LOGNOTICE, "stopping upnp");
  CUPnP::ReleaseInstance(bWait);

  return true;
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StartUPnPClient()
{
#ifdef HAS_UPNP

  CLog::Log(LOGNOTICE, "starting upnp controller");
  CUPnP::GetInstance()->StartClient();
  return IsUPnPClientRunning();
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::IsUPnPClientRunning()
{
#ifdef HAS_UPNP
  return CUPnP::GetInstance()->IsClientStarted();
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StopUPnPClient()
{
#ifdef HAS_UPNP
  if (!IsUPnPRendererRunning())
    return true;

  CLog::Log(LOGNOTICE, "stopping upnp client");
  CUPnP::GetInstance()->StopClient();

  return true;
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StartUPnPRenderer()
{
#ifdef HAS_UPNP
//  if (!CSettings::Get().GetBool("services.upnprenderer"))
    //return false;

  CLog::Log(LOGNOTICE, "starting upnp renderer");
  return CUPnP::GetInstance()->StartRenderer();
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::IsUPnPRendererRunning()
{
#ifdef HAS_UPNP
  return CUPnP::GetInstance()->IsInstantiated();
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StopUPnPRenderer()
{
#ifdef HAS_UPNP
  if (!IsUPnPRendererRunning())
    return true;

  CLog::Log(LOGNOTICE, "stopping upnp renderer");
  CUPnP::GetInstance()->StopRenderer();

  return true;
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StartUPnPServer()
{
#ifdef HAS_UPNP
  //if (!CSettings::Get().GetBool("services.upnpserver"))
  //  return false;

  CLog::Log(LOGNOTICE, "starting upnp server");
  return CUPnP::GetInstance()->StartServer();
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::IsUPnPServerRunning()
{
#ifdef HAS_UPNP
  return CUPnP::GetInstance()->IsInstantiated();
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StopUPnPServer()
{
#ifdef HAS_UPNP
  if (!IsUPnPRendererRunning())
    return true;

  CLog::Log(LOGNOTICE, "stopping upnp server");
  CUPnP::GetInstance()->StopServer();

  return true;
#endif // HAS_UPNP
  return false;
}

bool CNetworkServices::StartZeroconf()
{
#ifdef HAS_ZEROCONF
  if (IsZeroconfRunning())
    return true;

  LOG(INFO) << "starting zeroconf publishing";
  return CZeroconf::GetInstance()->Start();
#endif // HAS_ZEROCONF
  return false;
}

bool CNetworkServices::IsZeroconfRunning()
{
#ifdef HAS_ZEROCONF
  return CZeroconf::GetInstance()->IsStarted();
#endif // HAS_ZEROCONF
  return false;
}

bool CNetworkServices::StopZeroconf()
{
#ifdef HAS_ZEROCONF
  if (!IsZeroconfRunning())
    return true;

  LOG(INFO) << "stopping zeroconf publishing";
  CZeroconf::GetInstance()->Stop();

  return true;
#endif // HAS_ZEROCONF
  return false;
}

bool CNetworkServices::ValidatePort(int port)
{
  if (port <= 0 || port > 65535)
    return false;

#ifdef TARGET_LINUX
  if (!CUtil::CanBindPrivileged() && (port < 1024 || port > 65535))
    return false;
#endif

  return true;
}

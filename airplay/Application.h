#pragma once

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

#include "system.h" // for HAS_DVD_DRIVE et. al.


class CNetwork;

class CApplication
{
public:

  static CApplication& getInstance();
  enum ESERVERS {
    ES_WEBSERVER = 1,
    ES_AIRPLAYSERVER,
    ES_JSONRPCSERVER,
    ES_UPNPRENDERER,
    ES_UPNPSERVER,
    ES_EVENTSERVER,
    ES_ZEROCONF
  };

  CApplication(void);
  virtual ~CApplication(void);
  virtual bool Initialize();
  void StartServices();
  void StopServices();

  bool StartServer(enum ESERVERS eServer, bool bStart, bool bWait = false);

  void Stop(int exitCode);
  void RestartApp();

  bool IsMusicScanning() const;
  bool IsVideoScanning() const;
  CNetwork& getNetwork();

  typedef enum
  {
    PLAY_STATE_NONE = 0,
    PLAY_STATE_STARTING,
    PLAY_STATE_PLAYING,
    PLAY_STATE_STOPPED,
    PLAY_STATE_ENDED,
  } PlayState;
  PlayState m_ePlayState;
//  CCriticalSection m_playStateMutex;
protected:

  CNetwork    *m_network;
};

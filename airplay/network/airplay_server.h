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

#include "system.h"
#ifdef HAS_AIRPLAY

#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "base/threading/non_thread_safe.h"
#include "base/threading/thread.h"
#include "net/socket/tcp_listen_socket.h"

#include "utils/HttpParser.h"
#include "interfaces/IAnnouncer.h"

#include <string>
#include <map>
#include <list>

#include <sys/socket.h>
#define AIRPLAY_SERVER_VERSION_STR "130.14"

class CTCPClient;
class AirPlayListenSocket;
class CAirPlayServer
  : public base::Thread
  , public ANNOUNCEMENT::IAnnouncer
  , public net::StreamListenSocket::Delegate
  , public base::RefCountedThreadSafe<CAirPlayServer>
{
public:
  class Delegate {
  public:
    enum EVENTID{
      EVENT_DMSLIST_UPDATE = 0,      /* DMS列表更新 */
      EVENT_FILELIST_UPDATE,         /* 1文件列表更新 */
      EVENT_DMR_SETMUTE,             /*2 设置静音 */
      EVENT_DMR_SETVOLUME,           /* 3设置音量 */
      EVENT_DMR_GETMUTE,             /*4 获取静音状态 */
      EVENT_DMR_GETVOLUME,           /*5 获取音量 */
      EVENT_DMR_PLAY,                /* 6播放 */
      EVENT_DMR_PAUSE,               /*7 暂停 */
      EVENT_DMR_RESUME,              /* 8恢复 */
      EVENT_DMR_SEEK,                /* 9定位 */
      EVENT_DMR_STOP,                /* 10结束 */
      EVENT_DMR_GETPOSITIONINFO,     /*11 获取当前位置信息 */
      EVENT_DMR_GETTRANSPORTINFO,     /* 12获取Transport信息 dinglei*/
      EVENT_DMR_GETMEDIAINFO,     /* 13获取媒体信息 dinglei*/
      EVENT_DMR_NONE,                /*14 无效 */
      EVENT_DLNA_CHANNEL_RESOURCE_CHANGED, /*15共享频道播放冲突事件*/
      EVENT_DMR_SETTRANSFORMS_ZOOM, /*16设置展现的变换dinglei*/
      EVENT_DMR_GETPRODUCTINFO,/*17：得到与待播内容关联的产品列表dinglei*///roductInfo
      EVENT_DMR_ORDER /*订购*/
    };

    virtual int onEvent(EVENTID eventID, const std::string& eventArg) = 0;
    virtual int onEventSync(EVENTID eventID, const std::string& eventArg, std::string* out) = 0;
    virtual int PublishAirPlayService(
      const std::string& fcr_identifier,
      const std::string& fcr_type,
      const std::string& fcr_name,
      int f_port,
      std::vector<std::pair<std::string, std::string> > txt) = 0;
  };

  //AirPlayServer impl.
  static CAirPlayServer* Get();
  static bool StartServer(int port, bool nonlocal);
  static void StopServer(bool bWait);
  static bool isRunning();
  static bool SetCredentials(bool usePassword, const std::string& password);
  static bool IsPlaying(){ return m_isPlaying > 0;}
  static void backupVolume();
  static void restoreVolume();
  static int m_isPlaying;

  CAirPlayServer();
  // IAnnouncer IF
  virtual void Announce(ANNOUNCEMENT::AnnouncementFlag flag, const char *sender, const char *message, const std::string& json_data) OVERRIDE;
  //
  Delegate* SetDelegate(Delegate* delegate);
  void AnnounceToClients(int state, const std::string& receiver);
  void DisconnectClient(const std::string& sessionId);  
  bool Initialize(int port, bool nonlocal);
  void Deinitialize();

  // ListenSocketDelegate
  virtual void DidAccept(net::StreamListenSocket* server,
                         scoped_ptr<net::StreamListenSocket> connection) OVERRIDE;
  virtual void DidRead(net::StreamListenSocket* connection,
                       const char* data,
                       int len) OVERRIDE;
  virtual void DidClose(net::StreamListenSocket* sock) OVERRIDE;
  
  virtual ~CAirPlayServer();

protected:
  // Called just prior to starting the message loop
  virtual void Init();
  // Called to start the message loop
  virtual void Run(base::MessageLoop* message_loop);
  // Called just after the message loop ends
  virtual void CleanUp();
private:
  friend class CTCPClient;
  bool SetInternalCredentials(bool usePassword, const std::string& password);
  void AnnounceToClientsInThread(int state, const std::string& receiver);
  void DisconnectClientInThread(const std::string& sessionId);
  bool PublishService();
  void RequestSlides(const std::string& sessionId, const base::WeakPtr<CTCPClient>&);
  CTCPClient* FindConnectionBySocket(net::StreamListenSocket*);
  CTCPClient* FindConnectionBySessionId(std::string& sessionId);

  base::Lock m_connectionLock;
  typedef std::map<net::StreamListenSocket*, CTCPClient*> SocketToConnectionMap;
  SocketToConnectionMap m_connections;
  typedef std::map<std::string, net::StreamListenSocket*> SessionToConnectionMap;
  SessionToConnectionMap m_reverseEventSockets;
  SessionToConnectionMap m_reverseSlideSockets;
  int m_ServerSocket;
  int m_origVolume;
  int m_port;
  bool m_nonlocal;
  bool m_usePassword;
  std::string m_password;

  
  base::MessageLoop* m_loop;
  scoped_ptr<AirPlayListenSocket> service_listen_socket_;
  Delegate* m_delegate;

  base::WeakPtrFactory<CAirPlayServer> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(CAirPlayServer);
};

#endif

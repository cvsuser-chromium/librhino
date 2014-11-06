/*
 * Many concepts and protocol specification in this code are taken
 * from Airplayer. https://github.com/PascalW/Airplayer
 *
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
#include "base/bind.h"
#include "base/md5.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/values.h"
#include "base/json/json_writer.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_number_conversions.h"
#include "base/time/time.h"
#include "base/timer/timer.h"
#include "url/url_util.h"

#include "network/Network.h"
#include "network/Zeroconf.h"
#include "airplay_server.h"
#include "plist/plistcplusplus.h"
#include "settings/AdvancedSettings.h"

#ifdef HAS_AIRPLAY

#include "Application.h"
#include "interfaces/AnnouncementManager.h"

using namespace ANNOUNCEMENT;

#ifdef TARGET_WINDOWS
#define close closesocket
#ifdef DeleteFile
//#undef DeleteFile
#endif
#endif

#define RECEIVEBUFFER 1024
#define AUTH_REALM "AirPlay"
#define AUTH_REQUIRED "WWW-Authenticate: Digest realm=\""  AUTH_REALM  "\", nonce=\"%s\"\r\n"

namespace {
  enum EventState{
    EVENT_STATE_NONE     = -1,
    EVENT_STATE_PLAYING  =  0,
    EVENT_STATE_PAUSED   =  1,
    EVENT_STATE_LOADING  =  2,
    EVENT_STATE_STOPPED  =  3,
    EVENT_STATE_CLOSED   =  4,
  };
  base::LazyInstance<CAirPlayServer> gServerInstance = LAZY_INSTANCE_INITIALIZER;
  const char *eventStrings[] = {"playing", "paused", "loading", "stopped"};
  const char* kXAppleSessionId = "x-apple-session-id";
  const char* kXAppleTransition = "x-apple-transition";
  const char* kXAppleAssetKey = "x-apple-assetkey";
  const char* kXAppleAssetAction = "x-apple-assetaction";
  //const char* kXApplePurpose = "X-Apple-Purpose";
  const char* kXApplePurpose = "x-apple-purpose";
  const char* kAssetActionCacheOnly = "cacheonly";
  const char* kAssetActionDisplayCached = "displaycached";
  const char* kTransitionDissolve = "dissolve";
  static const char* kNullSessionId = "00000000-0000-0000-0000-000000000000";

  const char* kPlaybackInfo = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"\
                               "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"\
                               "<plist version=\"1.0\">\r\n"\
                               "<dict>\r\n"\
                               "<key>duration</key>\r\n<real>%f</real>\r\n"\
                               "<key>position</key>\r\n<real>%f</real>\r\n"\
                               "<key>rate</key>\r\n<real>%d</real>\r\n"\
                               "<key>readyToPlay</key> <true/>"\
                               "<key>loadedTimeRanges</key>\r\n"\
                               "<array>\r\n"\
                               "\t\t<dict>\r\n"\
                               "\t\t\t<key>duration</key>\t\t\t<real>%f</real>\r\n"\
                               "\t\t\t<key>start</key>\t\t\t<real>0.0</real>\r\n"\
                               "\t\t</dict>\r\n"\
                               "</array>\r\n"\
                               "<key>playbackBufferEmpty</key> <true/>\r\n"\
                               "<key>playbackBufferFull</key> <false/>\r\n"\
                               "<key>playbackLikelyToKeepUp</key><true/>\r\n"\
                               "<key>seekableTimeRanges</key>\r\n"\
                               "<array>\r\n"\
                               "\t\t<dict>\r\n"\
                               "\t\t\t<key>duration</key>\r\n"\
                               "\t\t\t<real>%f</real>\r\n"\
                               "\t\t\t<key>start</key>\r\n"\
                               "\t\t\t<real>0.0</real>\r\n"\
                               "\t\t</dict>\r\n"\
                               "</array>\r\n"\
                               "</dict>\r\n"\
                               "</plist>\r\n";

  const char* kPlaybackInfoNotReady = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"\
                                       "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"\
                                       "<plist version=\"1.0\">\r\n"\
                                       "<dict>\r\n"\
                                       "<key>readyToPlay</key>\r\n"\
                                       "<false/>\r\n"\
                                       "</dict>\r\n"\
                                       "</plist>\r\n";

  const char* kServerInfo = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"\
                             "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"\
                             "<plist version=\"1.0\">\r\n"\
                             "<dict>\r\n"\
                             "<key>deviceid</key>\r\n"\
                             "<string>%s</string>\r\n"\
                             "<key>features</key>\r\n"\
                             "<integer>8317</integer>\r\n"\
                             "<key>model</key>\r\n"\
                             "<string>hybroad,1</string>\r\n"\
                             "<key>protovers</key>\r\n"\
                             "<string>1.0</string>\r\n"\
                             "<key>srcvers</key>\r\n"\
                             "<string>"AIRPLAY_SERVER_VERSION_STR"</string>\r\n"\
                             "</dict>\r\n"\
                             "</plist>\r\n";

  const char* kEventInfo = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"\
                            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"\
                            "<plist version=\"1.0\">\r\n"\
                            "<dict>\r\n"\
                            "<key>category</key>\r\n"\
                            "<string>%s</string>\r\n"\
                            "<key>sessionID</key>\r\n"\
                            "<integer>%d</integer>\r\n"\
                            "<key>state</key>\r\n"\
                            "<string>%s</string>\r\n"\
                            "</dict>\r\n"\
                            "</plist>\r\n";


  const char* kSlideFeatures1 = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"\
                                 "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"\
                                 "<plist version=\"1.0\">\r\n"\
                                 "<dict>\r\n"\
                                 "<key>themes</key>\r\n"\
                                 "<array>\r\n"\
                                 "<dict>\r\n"\
                                 "<key>key</key>\r\n"\
                                 "<string>»ÃµÆÆ¬</string>\r\n"\
                                 "<key>name</key>\r\n"\
                                 "<string>»ÃµÆÆ¬</string>\r\n"\
                                 "</dict>\r\n"\
                                 "</array>\r\n"\
                                 "</dict>\r\n"\
                                 "</plist>\r\n";
  const char* kSlideFeatures = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"\
                                "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"\
                                "<plist version=\"1.0\"> "\
                                "<dict> "\
                                "<key>themes</key> "\
                                "<array> "\
                                "<dict> "\
                                "<key>key</key> "\
                                "<string>Reflections</string> "\
                                "<key>name</key> "\
                                "<string>Reflections</string> "\
                                "</dict> "\
                                "</array> "\
                                "</dict></plist> ";

  const char* kSlideShow = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"\
                            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\r\n"\
                            "<plist version=\"1.0\">\r\n"\
                            "<dict/>\r\n"\
                            "</plist>\r\n";

  //as of rfc 2617
  std::string calcResponse(const std::string& username,
    const std::string& password,
    const std::string& realm,
    const std::string& method,
    const std::string& digestUri,
    const std::string& nonce)
  {
    std::string response;
    std::string HA1;
    std::string HA2;
    std::string once = nonce;
    HA1 = base::MD5String(username + ":" + realm + ":" + password);
    HA2 = base::MD5String(method + ":" + digestUri);
    response = base::MD5String(StringToLowerASCII(HA1) + ":" + once + ":" + StringToLowerASCII(HA2));
    StringToLowerASCII(&response);
    return response;
    //return response.ToLower();
  }

  //helper function
  //from a string field1="value1", field2="value2" it parses the value to a field
  std::string getFieldFromString(const std::string &str, const char* field)
  {
#if 1
    bool next_is_option = false, next_is_value = false;
    std::string input = "text/html; charset=UTF-8; foo=bar";
    base::StringTokenizer t(input, ", =");
    t.set_options(base::StringTokenizer::RETURN_DELIMS);
    while (t.GetNext()) {
      if (t.token_is_delim()) {
        switch (*t.token_begin()) {
        case ';':
          next_is_option = true;
          break;
        case '=':
          next_is_value = true;
          break;
        }
      } else {
        const char* label;
        if (next_is_option) {
          label = "option-name";
          next_is_option = false;
        } else if (next_is_value) {
          label = "option-value";
          next_is_value = false;
        } else {
          label = "mime-type";
        }
        LOG(ERROR) << label << ": " <<  t.token().c_str();
      }
    }
#else
    CStdString tmpStr;
    CStdStringArray tmpAr1;
    CStdStringArray tmpAr2;

    StringUtils::SplitString(str, ",", tmpAr1);

    for(unsigned int i = 0;i<tmpAr1.size();i++)
    {
      if (tmpAr1[i].Find(field) != -1)
      {
        if (StringUtils::SplitString(tmpAr1[i], "=", tmpAr2) == 2)
        {
          tmpAr2[1].Remove('\"');//remove quotes
          return tmpAr2[1];
        }
      }
    }
#endif
    return "";
  }
}
class AirPlayListenSocket : public net::TCPListenSocket {
public:
  virtual ~AirPlayListenSocket() {}
  // static
  static scoped_ptr<AirPlayListenSocket> CreateAndListen(
    const std::string& ip, int port, StreamListenSocket::Delegate* del) {
    net::SocketDescriptor s = CreateAndBind(ip, port);
    if (s == net::kInvalidSocket)
      return scoped_ptr<AirPlayListenSocket>();
    scoped_ptr<AirPlayListenSocket> sock(new AirPlayListenSocket(s, del));
    sock->Listen();
    return sock.Pass();
  }
  // Get raw TCP socket descriptor bound to ip and return port it is bound to.
  static scoped_ptr<AirPlayListenSocket>
    CreateAndBindAnyPort(const std::string& ip, int* port, StreamListenSocket::Delegate* del){
      int local_port = 0;
      net::SocketDescriptor s = net::TCPListenSocket::CreateAndBindAnyPort(ip, &local_port);
      if (s == net::kInvalidSocket)
        return scoped_ptr<AirPlayListenSocket>();
      if (port)
        *port = local_port;
      scoped_ptr<AirPlayListenSocket> sock(new AirPlayListenSocket(s, del));
      sock->Listen();
      return sock.Pass();
    }
protected:
  AirPlayListenSocket(net::SocketDescriptor s, net::StreamListenSocket::Delegate* del)
    : net::TCPListenSocket(s, del){
    }
};

class CTCPClient
: public base::RefCounted<CTCPClient>
  , public NON_EXPORTED_BASE(base::NonThreadSafe) {
  public:
    enum SocketType{
      SOCKET_TYPE_NORMAL = 0,
      SOCKET_TYPE_REVERSE_EVENT,
      SOCKET_TYPE_REVERSE_SLIDESHOW,
    };
    enum RequestResult{
      REQUEST_RESULT_OK                         = 200,
      REQUEST_RESULT_SWITCHING_PROTOCOLS        = 101,
      REQUEST_RESULT_SWITCHING_PROTOCOLS_EVENT  = 102,
      REQUEST_RESULT_SWITCHING_PROTOCOLS_SLIDE  = 103,
      REQUEST_RESULT_NEED_AUTH                  = 401,
      REQUEST_RESULT_NOT_FOUND                  = 404,
      REQUEST_RESULT_METHOD_NOT_ALLOWED         = 405,
      REQUEST_RESULT_NOT_IMPLEMENTED            = 501,
      REQUEST_RESULT_NO_RESPONSE_NEEDED         = 1000
    };
    virtual ~CTCPClient();
    RequestResult PushBuffer(CAirPlayServer *host, const char *buffer,
      int length, std::string &sessionId );
    void ComposeReverseEvent(std::string& reverseHeader, std::string& reverseBody, int state);
    void Disconnect();

  private:
    friend class CAirPlayServer;
    CTCPClient(CAirPlayServer::Delegate* , scoped_ptr<net::StreamListenSocket>);
    RequestResult ProcessRequest(std::string& responseHeader, std::string& response);
    int ProcessPhotoRequest(std::string& responseHeader, std::string& responseBody);
    int ProcessPlayRequest(std::string& responseHeader, std::string& responseBody);
    int ProcessSlideShowRequest(std::string& responseHeader, std::string& responseBody);
    int ProcessReverseResponse(std::string& responseHeader, std::string& responseBody);
    bool checkAuthorization(const std::string& authStr, const std::string& method, const std::string& uri);
    void ComposeAuthRequestAnswer(std::string& responseHeader, std::string& responseBody);
    base::FilePath generateCachedFileName(std::string& assetId, const char buf[4]);

    void playPhoto(base::FilePath& file_path, std::string& transition);
    void startSlideShowIfNeeded();
    void scheduleNextSlide();
    void scheduleNextSlideRequest();
    std::string& sessionId() {return session_id_;}

    struct SlideShowParam{
      SlideShowParam()
        : slide_duration_(2),
        theme_("default"),
        state_(EVENT_STATE_NONE),
        last_asset_id_(0){
        }
      void reset(){
        slide_duration_ = 2;
        theme_ = "default";
        state_ = EVENT_STATE_NONE;
        last_asset_id_ = 0;
      }
      uint64_t slide_duration_;
      std::string theme_;
      EventState state_;
      uint64_t last_asset_id_;
    } slide_show_param_;
    struct SlideShowState{
      SlideShowState()
        :asset_id_(0),
        key_(0) {
        }
      uint64_t asset_id_;
      uint64_t key_;
      base::FilePath cached_file_;
    };
    std::queue<SlideShowState> slide_show_context_;
    int  last_event_;//m_lastEvent;
    int  session_counter_;//m_sessionCounter;
    bool is_authenticated_;//m_bAuthenticated;
    std::string auth_nonce_;//m_authNonce;
    scoped_ptr<HttpParser> http_parser_;//m_httpParser;
    CAirPlayServer::Delegate* delegate_;//m_delegate;
    std::list<base::FilePath> cached_files_;//m_cachedFiles;
    base::Lock lock_;//m_critSection;
    std::string session_id_;//m_sessionId;
    SocketType socket_type_;
    std::string playing_type_;
    base::Timer slide_show_timer_;
    scoped_ptr<net::StreamListenSocket> socket_;

    base::WeakPtrFactory<CTCPClient> weak_factory_;
    DISALLOW_COPY_AND_ASSIGN(CTCPClient);
  };

int CAirPlayServer::m_isPlaying = 0;

void CAirPlayServer::Announce(AnnouncementFlag flag, const char *sender, const char *message, const std::string& json_data)
{
  int error_code = -1;
  std::string err_message;
  std::string receiver;
  base::Value* value = base::JSONReader::ReadAndReturnError(json_data, 0, &error_code, &err_message);
  if (!value){
    LOG(WARNING) << "Got Wrong message format" << json_data;
    return ;
  }
  if (!value->IsType(base::Value::TYPE_DICTIONARY)) {
    //raise a exception to Java side.
  }
  base::DictionaryValue *message_dict = reinterpret_cast<base::DictionaryValue*>(value);
  message_dict->GetString("receiver", &receiver);

  if (receiver.empty())
    receiver = "all";

  if ((flag & Player) && strcmp(sender, "xbmc") == 0) {
    if (base::strcasecmp(message, "OnStop") == 0) {
      restoreVolume();
      CAirPlayServer::Get()->AnnounceToClients(EVENT_STATE_STOPPED, receiver);
    } else if (base::strcasecmp(message, "OnPlay") == 0) {
      CAirPlayServer::Get()->AnnounceToClients(EVENT_STATE_PLAYING, receiver);
    } else if (base::strcasecmp(message, "OnPause") == 0) {
      CAirPlayServer::Get()->AnnounceToClients(EVENT_STATE_PAUSED, receiver);
    } else if (base::strcasecmp(message, "OnDisconnect") == 0) {
      CAirPlayServer::Get()->AnnounceToClients(EVENT_STATE_STOPPED, receiver);
      CAirPlayServer::Get()->DisconnectClient(receiver);
    } else {
      LOG(ERROR_REPORT) << "unrecognized message " << message;
    }
  } else {
    LOG(ERROR_REPORT) << "unrecognized sender " << sender;
  }
}
CAirPlayServer::Delegate* CAirPlayServer::SetDelegate( Delegate* delegate)
{
  Delegate* old = m_delegate;
  m_delegate = delegate;
  return old;
}
void CAirPlayServer::AnnounceToClients(int state, const std::string& receiver)
{
  AnnounceToClientsInThread(state, receiver);
  //if (m_loop){
  //  m_loop->PostNonNestableTask(FROM_HERE,
  //    base::Bind(&CAirPlayServer::AnnounceToClientsInThread, weak_factory_.GetWeakPtr()));
  //}
}
void CAirPlayServer::DisconnectClient(const std::string& sessionId)
{
  if (m_loop){
    m_loop->PostNonNestableTask(FROM_HERE,
      base::Bind(&CAirPlayServer::DisconnectClientInThread, weak_factory_.GetWeakPtr(), sessionId));
  }
}
bool CAirPlayServer::Initialize(int port, bool nonlocal)
{
  Deinitialize();
  m_port = port;
  m_nonlocal = nonlocal;
  LOG(INFO) << "AIRPLAY Server: Successfully initialized";
  return true;
}

void CAirPlayServer::Deinitialize()
{
  base::AutoLock lock (m_connectionLock);

  SocketToConnectionMap::iterator it = m_connections.begin();
  for (; it != m_connections.end(); it++)
    it->second->Disconnect();

  m_connections.clear();
  m_reverseEventSockets.clear();
  m_reverseEventSockets.clear();
}

// ListenSocketDelegate
void CAirPlayServer::DidAccept(net::StreamListenSocket* server,
  scoped_ptr<net::StreamListenSocket> client)
{
  LOG(INFO) << "run here, socketHandle=" << socket;
  //DCHECK(server == server_) << "it's not the server socket??";
  if (server != static_cast<net::StreamListenSocket*>(service_listen_socket_.get()) ) {
    LOG(WARNING) << "it's not the server socket.";
    return ;
  }
  //net::StreamListenSocket* conn = connection.Pass();
  scoped_refptr<CTCPClient> tcp_client(new CTCPClient(m_delegate, client.Pass()));

  m_connections[tcp_client->socket_.get()] = tcp_client.get();
  tcp_client->AddRef();
}

void CAirPlayServer::DidRead(net::StreamListenSocket* socket,
  const char* data,
  int len)
{
  std::string sessionId;
  CTCPClient::RequestResult status(CTCPClient::REQUEST_RESULT_NOT_IMPLEMENTED);
  if (m_connections.find(socket) != m_connections.end()) {
    status = m_connections[socket]->PushBuffer(this, data, len, sessionId);
  }
  if (CTCPClient::REQUEST_RESULT_SWITCHING_PROTOCOLS_EVENT == status){
    m_reverseEventSockets[sessionId] = socket;

  }
  else if (CTCPClient::REQUEST_RESULT_SWITCHING_PROTOCOLS_SLIDE == status) {
    m_reverseSlideSockets[sessionId] = socket;
  }
}

void CAirPlayServer::DidClose(net::StreamListenSocket* socket)
{
  LOG(INFO) << "run here CAirPlayServer::DidClose.";
  SessionToConnectionMap::iterator it;
  std::string sessionId;
  if (m_connections.find(socket) != m_connections.end()) {
    sessionId = m_connections[socket]->sessionId();
    m_connections[socket]->Disconnect();
    m_connections[socket]->Release();
    m_connections.erase(socket);
  }
  it = m_reverseEventSockets.find(sessionId);
  if (it != m_reverseEventSockets.end()){
    m_reverseEventSockets.erase(it);
  }
  it = m_reverseSlideSockets.find(sessionId);
  if (it != m_reverseSlideSockets.end()){
    m_reverseSlideSockets.erase(it);
  }
}
bool CAirPlayServer::StartServer(int port, bool nonlocal)
{
  StopServer(true);

  if (gServerInstance.Pointer()->Initialize(port, nonlocal)) {
    Options ops(base::MessageLoop::TYPE_IO, 0);
    gServerInstance.Pointer()->StartWithOptions(ops);
    return true;
  }

  return false;
}
void CAirPlayServer::StopServer(bool bWait)
{
  if (bWait) {
    gServerInstance.Get().Stop();
  } else {
    gServerInstance.Get().StopSoon();
  }
}
bool CAirPlayServer::isRunning()
{
  return gServerInstance.Get().IsRunning();
}

bool CAirPlayServer::SetCredentials(bool usePassword, const std::string& password)
{
  return gServerInstance.Pointer()->SetInternalCredentials(usePassword, password);
}
void CAirPlayServer::backupVolume()
{
#if 0
  if (ServerInstance->m_origVolume == -1)
    ServerInstance->m_origVolume = (int)g_application.GetVolume();
#endif
}

void CAirPlayServer::restoreVolume()
{
#if 0
  if (ServerInstance->m_origVolume != -1 && CSettings::Get().GetBool("services.airplayvolumecontrol"))
  {
    g_application.SetVolume((float)ServerInstance->m_origVolume);
    ServerInstance->m_origVolume = -1;
  }
#endif
}
void CAirPlayServer::AnnounceToClientsInThread(int state, const std::string& receiver)
{
  base::AutoLock lock(m_connectionLock);
  SocketToConnectionMap::iterator it;
  net::StreamListenSocket* reverseSocket = 0;

  if (m_reverseEventSockets.empty()) {
    LOG(WARNING) << "no reverse socket yet!";
    return ;
  }

  std::string reverseHeader;
  std::string reverseBody;
  std::string response;

  for (it = m_connections.begin(); it != m_connections.end(); it++) {
    std::string& sessionId = it->second->sessionId();
    if (m_reverseEventSockets.find(sessionId) == m_reverseEventSockets.end()) {
      continue;
    }
    if (receiver == "all" || receiver == sessionId) {
      reverseSocket = m_reverseEventSockets[sessionId]; //that is our reverse socket
      if (reverseSocket == it->first){
        //don't send this myself.
        continue;
      }
      // Send event status per reverse http socket (play, loading, paused)
      // if we have a reverse header and a reverse socket
      {
        reverseHeader.clear();
        reverseBody.clear();
        response.clear();

        it->second->ComposeReverseEvent(reverseHeader, reverseBody, state);

        if (reverseHeader.length() <= 0 )
          continue;
        response = "POST /event HTTP/1.1\r\n";
        response += reverseHeader;
        response += "\r\n";

        if (reverseBody.size() > 0) {
          response += reverseBody;
        }
        if (reverseSocket)
          reverseSocket->Send(response, false);
      }
    }
  }
}

void CAirPlayServer::DisconnectClientInThread(const std::string& sessionId)
{
  base::AutoLock lock(m_connectionLock);
  SocketToConnectionMap::iterator it;
  net::StreamListenSocket* reverseSocket = 0;
  if (m_reverseEventSockets.empty()) {
    LOG(WARNING) << "no reverse socket yet!";
    return ;
  }
  std::vector<SocketToConnectionMap::iterator> tmpVector;
  for (it = m_connections.begin(); it != m_connections.end(); it++) {
    std::string& tmpId = it->second->sessionId();
    if (tmpId == sessionId){
      if (m_reverseEventSockets.find(sessionId) == m_reverseEventSockets.end()) {
        continue;
      }
      reverseSocket = m_reverseEventSockets[sessionId]; //that is our reverse socket
      if (reverseSocket == it->first){
        m_reverseEventSockets.erase(sessionId);
      }
      it->second->Disconnect();
      it->second->Release();
      tmpVector.push_back(it);
    }
  }
  for (size_t i=0; i < tmpVector.size(); i ++){
    m_connections.erase(tmpVector[i]);
  }
}
void CAirPlayServer::RequestSlides(const std::string& sessionId, const base::WeakPtr<CTCPClient>& client)
{
  if (!client) {
    LOG(ERROR_REPORT) << "Client has disconnected!";
    return ;
  }
  if (m_reverseSlideSockets.find(sessionId) == m_reverseSlideSockets.end()){
    LOG(ERROR_REPORT) << "not create slide show connection yet!";
    return;
  }

  {
    std::string request;
    net::StreamListenSocket* reverseSocket = 0;

    request = "GET /slideshows/1/assets/1 HTTP/1.1\r\n";
    request += "Accept: application/x-apple-binary-plist\r\n";
    request += "Content-Length: 0\r\n";
    request += "X-Apple-Session-ID: " + sessionId;

    request += "\r\n\r\n";

    reverseSocket = m_reverseSlideSockets[client->sessionId()];
    if (reverseSocket)
      reverseSocket->Send(request, false);
  }
}

CTCPClient* CAirPlayServer::FindConnectionBySocket(net::StreamListenSocket* socket)
{
  SocketToConnectionMap::iterator it = m_connections.find(socket);
  if (it == m_connections.end())
    return NULL;
  return (it->second);
}

CAirPlayServer::CAirPlayServer()
: Thread("AirPlayServer")
  , weak_factory_(this)
{
  m_ServerSocket = INVALID_SOCKET;
  m_usePassword = false;
  m_origVolume = -1;
  CAnnouncementManager::AddAnnouncer(this);
}

CAirPlayServer::~CAirPlayServer()
{
  CAnnouncementManager::RemoveAnnouncer(this);
}
CAirPlayServer* CAirPlayServer::Get()
{
  return gServerInstance.Pointer();
}

void CAirPlayServer::Init()
{
  std::string ip;

  if (m_nonlocal)
    ip = "0.0.0.0";
  else
    ip = "127.0.0.1";
  service_listen_socket_ = AirPlayListenSocket::CreateAndListen(ip, m_port, this);

  if (!service_listen_socket_) {
    service_listen_socket_ = AirPlayListenSocket::CreateAndBindAnyPort(ip, &m_port, this);
  }

  PublishService();
  m_loop = base::MessageLoop::current();

}
void CAirPlayServer::Run(base::MessageLoop* message_loop)
{
  Thread::Run(message_loop);
}

void CAirPlayServer::CleanUp()
{
}
bool CAirPlayServer::SetInternalCredentials(bool usePassword, const std::string& password)
{
  m_usePassword = usePassword;
  m_password = password;
  return true;
}

bool CAirPlayServer::PublishService()
{
  int ret = -1;
  const char* kFcr_identifier = "servers.airplay";
  const char* kFr_type = "_airplay._tcp";
  LOG(INFO) << "PublishService Airplay";
  std::vector<std::pair<std::string, std::string> > txt;
  CNetworkInterface* iface = CApplication::getInstance().getNetwork().GetFirstConnectedInterface();
  txt.push_back(std::make_pair("deviceid", iface != NULL ? iface->GetMacAddress() : "FF:FF:FF:FF:FF:F2"));
  //txt.push_back(std::make_pair("features", "0x207D"));
  txt.push_back(std::make_pair("features", "0x211B"));

  txt.push_back(std::make_pair("model", "hybroad,1"));
  txt.push_back(std::make_pair("srcvers", AIRPLAY_SERVER_VERSION_STR));

  std::string friedlyname;
  friedlyname.append(iface != NULL ? iface->GetMacAddress() : "FF:FF:FF:FF:FF:F2");
  friedlyname.append("@hybroad");

  if (m_delegate) {
    LOG(INFO) << "PublishService Airplay";
    ret = m_delegate->PublishAirPlayService(kFcr_identifier, kFr_type, friedlyname, m_port, txt);
  }
  if (ret < 0) {
    LOG(INFO) << "PublishService Airplay";
    CZeroconf::GetInstance()->PublishService(kFcr_identifier, kFr_type, friedlyname, m_port, txt);
  }
  return true;
}
CTCPClient::CTCPClient(CAirPlayServer::Delegate* delegate, scoped_ptr<net::StreamListenSocket> socket)
: weak_factory_(this),
  slide_show_timer_(true, false), /*we need it remain the task, not repeating.*/
  socket_(socket.Pass())
{
  static int sessionCounter = 0;
  http_parser_.reset(new HttpParser());
  is_authenticated_ = false;
  last_event_ = EVENT_STATE_NONE;
  delegate_ = delegate;
  socket_type_ = SOCKET_TYPE_NORMAL;

  sessionCounter++;
  session_counter_ = sessionCounter;
}
CTCPClient::~CTCPClient()
{
  Disconnect();
}

CTCPClient::RequestResult
CTCPClient::PushBuffer(CAirPlayServer *host, const char *buffer,
  int length, std::string &sessionId)
{
  HttpParser::status_t status = http_parser_->addBytes(buffer, length);
  RequestResult result(REQUEST_RESULT_NOT_IMPLEMENTED);
  if (status == HttpParser::Done) {
    // Parse the request
    std::string responseHeader;
    std::string responseBody;
    result = ProcessRequest(responseHeader, responseBody);
    sessionId = session_id_;
    CStdString statusMsg = "OK";
    int statusCode = result;
    switch (result) {
    case REQUEST_RESULT_NOT_IMPLEMENTED:
      statusMsg = "Not Implemented";
      break;
    case REQUEST_RESULT_SWITCHING_PROTOCOLS_SLIDE:
    case REQUEST_RESULT_SWITCHING_PROTOCOLS_EVENT:
      statusMsg = "Switching Protocols";
      statusCode = REQUEST_RESULT_SWITCHING_PROTOCOLS;
      //reverseSockets[sessionId] = socket_;//save this socket as reverse http socket for this sessionid
      break;
    case REQUEST_RESULT_NEED_AUTH:
      statusMsg = "Unauthorized";
      break;
    case REQUEST_RESULT_NOT_FOUND:
      statusMsg = "Not Found";
      break;
    case REQUEST_RESULT_METHOD_NOT_ALLOWED:
      statusMsg = "Method Not Allowed";
      break;
    }

    // Prepare the response
    std::string response;
    const time_t ltime = time(NULL);
    char *date = asctime(gmtime(&ltime)); //Fri, 17 Dec 2010 11:18:01 GMT;
    date[strlen(date) - 1] = '\0'; // remove \n
    //response.Format("HTTP/1.1 %d %s\nDate: %s\r\n", status, statusMsg.c_str(), date);
    base::StringAppendF(&response, "HTTP/1.1 %d %s\nDate: %s GMT+00:00\r\n", statusCode, statusMsg.c_str(), date);
    if (responseHeader.size() > 0) {
      response += responseHeader;
    }

    if (responseBody.size() > 0) {
      //response.Format("%sContent-Length: %d\r\n", response.c_str(), responseBody.size());
      base::StringAppendF(&response, "Content-Length: %d\r\n", responseBody.size());
    }
    response += "\r\n";

    if (responseBody.size() > 0) {
      response += responseBody;
    }

    // Send the response
    //don't send response on AIRPLAY_STATUS_NO_RESPONSE_NEEDED
    if (result != REQUEST_RESULT_NO_RESPONSE_NEEDED) {
      //send(socket_, response.c_str(), response.size(), 0);
      socket_->Send(response, false);
    }
    // We need a new parser...
    http_parser_.reset(new HttpParser);
  }
  return result;
}

void CTCPClient::Disconnect()
{
  if (socket_) {
    http_parser_.reset(0);
    //removed local cache files here.
    std::list<base::FilePath>::iterator it = cached_files_.begin();
    for ( ;it != cached_files_.end(); it++){
      base::DeleteFile(*it, false);
      LOG(ERROR_REPORT) << "delete file: " << it->value();
    }
    cached_files_.clear();
  }
}

void CTCPClient::ComposeReverseEvent( std::string& reverseHeader,
  std::string& reverseBody,
  int state)
{
  PList::Node* node = PList::Structure::FromXml(kEventInfo);
  if (last_event_ != state
    || (EVENT_STATE_PLAYING == last_event_ && state == EVENT_STATE_PLAYING)) {
    if (node && node->GetType() == PLIST_DICT ){
      PList::Dictionary* dict_node = (static_cast<PList::Dictionary*>(node));

      switch(state) {
      case EVENT_STATE_PLAYING:
      case EVENT_STATE_PAUSED:
        if (playing_type_ == "photo" || playing_type_ == "slideshow") {
          PList::Integer insertValue;
          if ((*dict_node)["sessionID"]) {
            static_cast<PList::Integer*>((*dict_node)["sessionID"])->SetValue(session_counter_);
          } else {
            insertValue.SetValue(session_counter_);
            dict_node->Insert("sessionID", &insertValue);
          }
          if ((*dict_node)["lastAssetID"]) {
            static_cast<PList::Integer*>((*dict_node)["lastAssetID"])->SetValue(slide_show_param_.last_asset_id_);
          } else {
            insertValue.SetValue(slide_show_param_.last_asset_id_);
            dict_node->Insert("lastAssetID", &insertValue);
          }
        } else {
        }
      case EVENT_STATE_STOPPED:
      case EVENT_STATE_LOADING:
        static_cast<PList::String*>((*dict_node)["category"])->SetValue(playing_type_);
        static_cast<PList::String*>((*dict_node)["state"])->SetValue(eventStrings[state]);
        //reverseBody.Format(kEventInfo, session_counter, eventStrings[state]);
        //base::StringAppendF(&reverseBody, kEventInfo, playing_type_.c_str(), session_counter_, eventStrings[state]);
        reverseBody = dict_node->ToXml();
        //reverseBody += "\r\n";
        LOG(INFO) << "AIRPLAY: sending event: " << eventStrings[state];
        break;
      case EVENT_STATE_NONE:
      default:
        return;
      }
    }
    reverseHeader = "Content-Type: text/x-apple-plist+xml\r\n";
    base::StringAppendF(&reverseHeader, "Content-Length: %d\r\n", reverseBody.length());
    base::StringAppendF(&reverseHeader, "x-apple-session-id: %s\r\n", session_id_.c_str());
    last_event_ = state;
  }
}

void CTCPClient::ComposeAuthRequestAnswer(std::string& responseHeader, std::string& responseBody)
{
  std::string randomStr;
  int16_t random = rand();
  randomStr = random;
  auth_nonce_ = base::MD5String(randomStr);
  //responseHeader.Format(AUTH_REQUIRED,m_authNonce);
  base::StringAppendF(&responseHeader, AUTH_REQUIRED, auth_nonce_.c_str());
  responseBody.clear();
}

bool CTCPClient::checkAuthorization(const std::string& authStr,
  const std::string& method,
  const std::string& uri)
{
  bool authValid = true;
  std::string username;

  if (authStr.empty())
    return false;

  //first get username - we allow all usernames for airplay (usually it is AirPlay)
  username = getFieldFromString(authStr, "username");
  if (username.empty()) {
    authValid = false;
  }

  //second check realm
  if (authValid) {
    if (getFieldFromString(authStr, "realm") != AUTH_REALM) {
      authValid = false;
    }
  }

  //third check nonce
  if (authValid) {
    if (getFieldFromString(authStr, "nonce") != auth_nonce_) {
      authValid = false;
    }
  }

  //forth check uri
  if (authValid) {
    if (getFieldFromString(authStr, "uri") != uri) {
      authValid = false;
    }
  }

  //last check response
  if (authValid) {
    std::string realm = AUTH_REALM;
    std::string ourResponse = calcResponse(username, gServerInstance.Get().m_password, realm, method, uri, auth_nonce_);
    std::string theirResponse = getFieldFromString(authStr, "response");
    if (LowerCaseEqualsASCII(ourResponse, theirResponse.c_str())) {
      authValid = false;
      LOG(WARNING) << "AirAuth: response mismatch - our: "<< ourResponse.c_str() << " theirs: " << theirResponse.c_str();
    } else {
      LOG(WARNING) << "AirAuth: successfull authentication from AirPlay client";
    }
  }
  is_authenticated_ = authValid;
  return is_authenticated_;
}
base::FilePath CTCPClient::generateCachedFileName(std::string& assetId, const char buf[4])
{
  base::FilePath tmpFile;
  base::PlatformFileError cerror;
#if defined(OS_WIN)
  file_util::GetTempDir(&tmpFile);
#else
  tmpFile = base::FilePath::FromUTF8Unsafe("/var");
#endif
  tmpFile = tmpFile.AppendASCII("airplay_photo");

  tmpFile = tmpFile.AppendASCII(assetId);
  tmpFile = tmpFile.ReplaceExtension(FILE_PATH_LITERAL(".jpg"));

  if (buf[1] == 'P' && buf[2] == 'N' && buf[3] == 'G') {
    tmpFile = tmpFile.ReplaceExtension(FILE_PATH_LITERAL(".png"));
  } else if (buf[0] == 'G' && buf[1] == 'I' && buf[2] == 'F'){
    tmpFile = tmpFile.ReplaceExtension(FILE_PATH_LITERAL(".gif"));
  }

  file_util::CreateDirectoryAndGetError(tmpFile.DirName(), &cerror);
  return tmpFile;
}
void CTCPClient::playPhoto(base::FilePath& file_path, std::string& transition)
{
  DictionaryValue play_dict;
  bool isTrue = true;
  play_dict.Set("instanceID", new base::FundamentalValue(0));
  play_dict.Set("PlayUrl", new StringValue(file_path.AsUTF8Unsafe()));
  play_dict.Set("mediaType", new StringValue("PICTURE"));
  play_dict.Set("slidedShow", new base::FundamentalValue(isTrue));
  play_dict.Set("transitionType", new StringValue(transition));
  play_dict.Set("autoDelete", new base::FundamentalValue(isTrue));
  play_dict.Set("sender", new StringValue(session_id_));
  std::string eventArg;
  base::JSONWriter::Write(&play_dict, &eventArg);
  if (delegate_)
    delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_PLAY, eventArg);
}
void CTCPClient::startSlideShowIfNeeded()
{
  if (slide_show_timer_.IsRunning())
    return;
  scheduleNextSlide();
}
void CTCPClient::scheduleNextSlide()
{
  if (!slide_show_context_.empty()) {
    playPhoto(slide_show_context_.front().cached_file_, slide_show_param_.theme_);
    cached_files_.push_back(slide_show_context_.front().cached_file_);
    slide_show_param_.last_asset_id_ = slide_show_context_.front().asset_id_;
    slide_show_context_.pop();
    slide_show_timer_.Start(FROM_HERE,
      base::TimeDelta::FromSeconds(slide_show_param_.slide_duration_),
      base::Bind(&CTCPClient::scheduleNextSlide, weak_factory_.GetWeakPtr()));
  }
}
void CTCPClient::scheduleNextSlideRequest()
{
  gServerInstance.Pointer()->AddRef();
  base::MessageLoop::current()->PostTask(FROM_HERE,
    base::Bind(&CAirPlayServer::RequestSlides, gServerInstance.Pointer(), session_id_, weak_factory_.GetWeakPtr()));
}
int CTCPClient::ProcessPhotoRequest(std::string& responseHeader,
  std::string& responseBody)
{
  //cached filename
  std::string assetAction = http_parser_->getValue(kXAppleAssetAction);
  std::string assetKey = http_parser_->getValue(kXAppleAssetKey);
  std::string transition = http_parser_->getValue(kXAppleTransition);
  bool isTrue = true;

  if (LowerCaseEqualsASCII(assetAction, kAssetActionDisplayCached)){
    if (assetKey.empty()) {
      LOG(ERROR_REPORT) << "it isn't cached.";
      return -1;
    }
    std::list<base::FilePath>::iterator it = cached_files_.begin();
    std::size_t found = 0;
    for (; it != cached_files_.end(); it++){
      std::string filename = it->AsUTF8Unsafe();
      found = filename.find(assetKey);
      if (found != std::string::npos){
        playPhoto(*it, transition);
        return 0;
      }
    }
    LOG(ERROR_REPORT) << "can't find cached photo with asset id " << assetKey;
    return -1;
  }
  if (http_parser_->getContentLength() > 0) {
    int writtenBytes=0;
    char buf[4];

    buf[0] = http_parser_->getBody()[0];
    buf[1] = http_parser_->getBody()[1];
    buf[2] = http_parser_->getBody()[2];
    buf[3] = http_parser_->getBody()[3];

    if (assetKey.empty()) {
      base::SStringPrintf(&assetKey, "%s_%d", session_id_.c_str(), session_counter_);
    }
    base::FilePath tmpFile = generateCachedFileName(assetKey, buf);
    writtenBytes = file_util::WriteFile(tmpFile, http_parser_->getBody(), http_parser_->getContentLength());
    if (writtenBytes != http_parser_->getContentLength()){
      LOG(ERROR_REPORT) << "AirPlayServer: Error writing tmpFile: " << tmpFile.AsUTF8Unsafe();
      return -1;
    }
    if (!LowerCaseEqualsASCII(assetAction, kAssetActionCacheOnly)) {
      playPhoto(tmpFile, transition);
    }
    LOG(INFO) << "AirPlayServer: writing tmpFile: " << tmpFile.AsUTF8Unsafe();
    cached_files_.push_back(tmpFile);
  }
  playing_type_ = "photo";
  return 0;
}

int CTCPClient::ProcessPlayRequest(std::string& responseHeader,
  std::string& responseBody)
{
  std::string body = http_parser_->getBody();
  std::string contentType = http_parser_->getValue("content-type");
  std::string location;
  double position = 0.0;
  last_event_ = EVENT_STATE_NONE;


  if (contentType == "application/x-apple-binary-plist") {
    CAirPlayServer::m_isPlaying++;

    const char* bodyChr = http_parser_->getBody();

    plist_t dict = NULL;
    plist_from_bin(bodyChr, http_parser_->getContentLength(), &dict);

    if (plist_dict_get_size(dict)) {
      plist_t tmpNode = plist_dict_get_item(dict, "Start-Position");
      if (tmpNode) {
        double tmpDouble = 0.0;
        plist_get_real_val(tmpNode, &tmpDouble);
        position = tmpDouble;
        DLOG(INFO) << "Start-Position=" << tmpDouble;
      }
      DLOG(INFO) << "Start-Position=" << position;
      tmpNode = plist_dict_get_item(dict, "Content-Location");
      if (tmpNode) {
        char *tmpStr = NULL;
        plist_get_string_val(tmpNode, &tmpStr);
        location = tmpStr;
        free(tmpStr);
      }
#if 1
      {
        PList::Dictionary param_dict(dict);
        PList::Dictionary::iterator iter = param_dict.Begin();
        PList::Node* node = NULL;
        std::string sub_key;

        //param_dict's responsible for destory the plist_t, so wo forget it.
        dict = NULL;

        while( iter != param_dict.End()){
          node = iter->second;
          sub_key = iter->first;
          switch( node->GetType()){
          case PLIST_BOOLEAN:
            LOG(INFO) << "PLIST_BOOLEAN key=" << iter->first << " value=" << static_cast<PList::Boolean*>(node)->GetValue();
            break;
          case PLIST_UINT:
            LOG(INFO) << "PLIST_UINT key=" << iter->first << " value=" << static_cast<PList::Integer*>(node)->GetValue();
            break;
          case PLIST_REAL:
            LOG(INFO) << "PLIST_REAL key=" << iter->first << " value=" << static_cast<PList::Real*>(node)->GetValue();
            break;
          case PLIST_STRING:
            LOG(INFO) << "PLIST_STRING key=" << iter->first << " value=" << static_cast<PList::String*>(node)->GetValue();
            break;
          case PLIST_KEY:
            break;
          case PLIST_DATA:
            break;
          case PLIST_DATE:
            {
              timeval tv = static_cast<PList::Date*>(node)->GetValue();
              LOG(INFO) << "PLIST_DATE key=" << iter->first << " time.tv_sec=" << tv.tv_sec << " time.tv_usec=" << tv.tv_usec;
            }
            break;
          case PLIST_ARRAY:
            break;
          case PLIST_DICT:
            break;
          case PLIST_NONE:
          default:
            break;
          }
          iter ++;
        }
#endif
      }
      if (dict) {
        plist_free(dict);
      }
    } else {
      LOG(ERROR_REPORT) << "Error parsing plist";
    }
  } else {
    CAirPlayServer::m_isPlaying++;
    // Get URL to play
    size_t start = body.find("Content-Location: ");
    if (start == std::string::npos)
      return REQUEST_RESULT_NOT_IMPLEMENTED;
    start += strlen("Content-Location: ");
    size_t end = body.find('\n', start);
    //location = body.Mid(start, end - start);
    location.assign(body, start, end - start);
    start = body.find("Start-Position");
    if (start != std::string::npos) {
      start += strlen("Start-Position: ");
      size_t end = body.find('\n', start);
      std::string positionStr(body, start, end - start);
      //position = (float)atof(positionStr.c_str());
      base::StringToDouble(positionStr, &position);
    }
  }

  static const char* kUserAgent="AppleCoreMedia/1.0.0.8F455 (AppleTV; U; CPU OS 4_3 like Mac OS X; de_de)";

  DictionaryValue play_dict;
  {
    DictionaryValue* header_dict = new DictionaryValue();
    header_dict->Set("UserAgent", new StringValue(kUserAgent));
    play_dict.Set("Header", header_dict);
  }
  play_dict.Set("instanceID", new base::FundamentalValue(0));

  play_dict.Set("PlayUrl", new StringValue(location));
  play_dict.Set("mediaType", new StringValue("M_VIDEO"));
  play_dict.Set("speed", new base::FundamentalValue(1));
  play_dict.Set("sender", new StringValue(session_id_));
  if (position != 0.0) {
    play_dict.Set("StartPercent", new base::FundamentalValue(position*100));
    LOG(INFO) << "StartPercent=" << position*100;
  }

  std::string eventArg;
  base::JSONWriter::Write(&play_dict, &eventArg);
  LOG(INFO) << "play command: " << eventArg;
  if (delegate_)
    delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_PLAY, eventArg);
  playing_type_ = "video";
  responseBody = "";
  gServerInstance.Get().AnnounceToClients(EVENT_STATE_LOADING, "all");

  return 0;
}
int CTCPClient::ProcessSlideShowRequest(std::string& responseHeader,
  std::string& responseBody)
{
  LOG(INFO) << "run here.";
  std::string body = http_parser_->getBody();
  std::string contentType = http_parser_->getValue("content-type");
  plist_t dict = NULL;

  if (contentType == "application/x-apple-binary-plist") {
    const char* bodyChr = http_parser_->getBody();
    plist_from_bin(bodyChr, http_parser_->getContentLength(), &dict);
  } else if (contentType == "text/x-apple-plist+xml") {
    const char* bodyChr = http_parser_->getBody();
    plist_from_xml(bodyChr, http_parser_->getContentLength(), &dict);
  }

  if (!dict) {
    LOG(ERROR_REPORT) << "Error parsing plist: " << http_parser_->getBody();
  }
  {
    PList::Dictionary param_dict(dict);

    PList::Node* node = NULL;
    std::string sub_key;

    //param_dict's responsible for destory the plist_t, so wo forget it.
    dict = NULL;
    node = param_dict["settings"];
    if (!node || node->GetType() != PLIST_DICT){
      return -1;
    }
    PList::Dictionary* settings_dict = static_cast<PList::Dictionary*>(node);
    PList::Dictionary::iterator iter = settings_dict->Begin();
    while (iter != settings_dict->End()) {
      node = iter->second;
      sub_key = iter->first;
      switch( node->GetType()){
      case PLIST_UINT:
        LOG(INFO) << "PLIST_UINT key=" << iter->first << " value=" << static_cast<PList::Integer*>(node)->GetValue();
        if (LowerCaseEqualsASCII(iter->first, "slideduration")) {
          slide_show_param_.slide_duration_ = static_cast<PList::Integer*>(node)->GetValue();
        }
        break;
      case PLIST_REAL:
        LOG(INFO) << "PLIST_REAL key=" << iter->first << " value=" << static_cast<PList::Real*>(node)->GetValue();
        break;
      case PLIST_STRING:
        LOG(INFO) << "PLIST_STRING key=" << iter->first << " value=" << static_cast<PList::String*>(node)->GetValue();
        if (LowerCaseEqualsASCII(iter->first, "theme")) {
          slide_show_param_.theme_ = static_cast<PList::String*>(node)->GetValue();
        }
        break;
      case PLIST_DATE:
        {
          timeval tv = static_cast<PList::Date*>(node)->GetValue();
          LOG(INFO) << "PLIST_DATE key=" << iter->first << " time.tv_sec=" << tv.tv_sec << " time.tv_usec=" << tv.tv_usec;
        }
        break;
      case PLIST_ARRAY:
      case PLIST_DICT:
      case PLIST_KEY:
      case PLIST_DATA:
      case PLIST_BOOLEAN:
      case PLIST_NONE:
      default:
        break;
      }
      iter ++;
    }
    if (dict) {
      plist_free(dict);
    }
  }
  gServerInstance.Get().AnnounceToClients(EVENT_STATE_LOADING, "all");
  scheduleNextSlideRequest();
  responseBody = kSlideShow;
  playing_type_ = "slideshow";
  return 0;
}
int CTCPClient::ProcessReverseResponse(std::string& reponseHeader,
  std::string& resonseBody)
{
  std::string contentType = http_parser_->getValue("content-type");
  SlideShowState slide_show_state;
  plist_t dict = NULL;

  std::string uri = http_parser_->getUri();
  if (uri == "204"){
    LOG(INFO) << "slide show finish.";
    slide_show_context_.push(slide_show_state);
    return 0;
  }
  if (http_parser_->getContentLength() <= 0 )
    return 0;

  if (contentType == "application/x-apple-binary-plist") {
    const char* bodyChr = http_parser_->getBody();
    plist_t dict = NULL;
    plist_from_bin(bodyChr, http_parser_->getContentLength(), &dict);
    {
      PList::Dictionary param_dict(dict);
      PList::Dictionary::iterator iter = param_dict.Begin();
      PList::Node* node = NULL;
      std::string asset_key = session_id_;
      PList::Node* pic_data_node = NULL;
      //param_dict's responsible for destory the plist_t, so wo forget it.
      dict = NULL;

      while( iter != param_dict.End()){
        node = iter->second;
        //sub_key = iter->first;
        switch( node->GetType()){
        case PLIST_DATA:
          LOG(INFO) << "PLIST_STRING key=" << iter->first;
          if (LowerCaseEqualsASCII(iter->first, "data")){
            pic_data_node = node;
          }
          break;
        case PLIST_DICT:
          if (LowerCaseEqualsASCII(iter->first, "info")){
            PList::Dictionary& info_dict = *(static_cast<PList::Dictionary*>(node));
            PList::Node *info_sub_node = info_dict["id"];
            if (info_sub_node){
              if (info_sub_node->GetType() == PLIST_UINT) {
                slide_show_state.asset_id_ = static_cast<PList::Integer*>(info_sub_node)->GetValue();
              } else if (info_sub_node->GetType() == PLIST_STRING) {
                base::StringToUint64(static_cast<PList::String*>(info_sub_node)->GetValue(), &slide_show_state.asset_id_);
              }
            }
            info_sub_node = info_dict["key"];
            if (info_sub_node){
              if (info_sub_node->GetType() == PLIST_UINT) {
                slide_show_state.key_ = static_cast<PList::Integer*>(info_sub_node)->GetValue();
              } else if (info_sub_node->GetType() == PLIST_STRING) {
                base::StringToUint64(static_cast<PList::String*>(info_sub_node)->GetValue(), &slide_show_state.key_);
              }
            }
          }
          break;
        case PLIST_BOOLEAN:
        case PLIST_UINT:
        case PLIST_REAL:
        case PLIST_STRING:
        case PLIST_KEY:
        case PLIST_DATE:
        case PLIST_ARRAY:
        case PLIST_NONE:
        default:
          break;
        }
        iter ++;
      }

      if (pic_data_node) {
        int writtenBytes = 0;
        std::vector<char> data = static_cast<PList::Data*>(pic_data_node)->GetValue();
        char buf[4];
        buf[0] = data[0];
        buf[1] = data[1];
        buf[2] = data[2];
        buf[3] = data[3];
        std::string assetId;
        base::SStringPrintf(&assetId, "%s_%"PRId64, session_id_.c_str(), slide_show_state.asset_id_);
        base::FilePath tmpFile = generateCachedFileName(assetId, buf);
        //TODO(teddy): should check data size here.
        //if (data.size() <= 0)
        writtenBytes = file_util::WriteFile(tmpFile, &data[0], data.size());
        if (writtenBytes != data.size()){
          LOG(ERROR_REPORT) << "AirPlayServer: Error writing tmpFile: " << tmpFile.AsUTF8Unsafe();
          return -1;
        }
        slide_show_state.cached_file_ = tmpFile;
        slide_show_context_.push(slide_show_state);
        startSlideShowIfNeeded();
      }
    }
    if (dict) {
      plist_free(dict);
    }
  }
  scheduleNextSlideRequest();
  return 0;
}

CTCPClient::RequestResult
CTCPClient::ProcessRequest(std::string& responseHeader,
  std::string& responseBody)
{
  std::string method = http_parser_->getMethod();
  std::string uri = http_parser_->getUri();
  std::string queryString = http_parser_->getQueryString();
  std::string body = http_parser_->getBody();
  std::string contentType = http_parser_->getValue("content-type");
  std::string authorization = http_parser_->getValue("authorization");

  RequestResult status = REQUEST_RESULT_OK;
  bool needAuth = false;

  if (session_id_.empty() || session_id_ == kNullSessionId) {
    session_id_ = http_parser_->getValue(kXAppleSessionId);
  }

  if (gServerInstance.Get().m_usePassword && !is_authenticated_) {
    needAuth = true;
  }
  size_t startQs = uri.find('?');
  if (startQs != std::string::npos) {
    //uri = uri.Left(startQs);
    uri.erase(startQs);
  }

  // This is the socket which will be used for reverse HTTP
  // negotiate reverse HTTP via upgrade
  if (uri == "/reverse") {
    status = REQUEST_RESULT_SWITCHING_PROTOCOLS_EVENT;
    socket_type_ = SOCKET_TYPE_REVERSE_EVENT;
    responseHeader = "Upgrade: PTTH/1.0\r\nConnection: Upgrade\r\n";
    std::string purpose = http_parser_->getValue(kXApplePurpose);
    if (LowerCaseEqualsASCII( purpose, "slideshow")){
      socket_type_ = SOCKET_TYPE_REVERSE_SLIDESHOW;
      status = REQUEST_RESULT_SWITCHING_PROTOCOLS_SLIDE;
    }
  }

  // The rate command is used to play/pause media.
  // A value argument should be supplied which indicates media should be played or paused.
  // 0.000000 => pause
  // 1.000000 => play
  else if (uri == "/rate") {
    const char* found = strstr(queryString.c_str(), "value=");
    int rate = found ? (int)(atof(found + strlen("value=")) + 0.5f) : 0;

    DictionaryValue rate_dict;
    std::string eventArg;
    rate_dict.Set("instanceID", new base::FundamentalValue(0));
    LOG(INFO) << "AIRPLAY: got request "<< uri.c_str() << " with rate " << rate;
    rate_dict.Set("rate", new base::FundamentalValue(rate));
    base::JSONWriter::Write(&rate_dict, &eventArg);

    if (delegate_) {
      if (rate == 0) { //pause
        delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_PAUSE, eventArg);
      } else { //play
        delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_RESUME, eventArg);
      }
    }
  }

  // The volume command is used to change playback volume.
  // A value argument should be supplied which indicates how loud we should get.
  // 0.000000 => silent
  // 1.000000 => loud
  else if (uri == "/volume")
  {
    static const float dBConvertInverse = 1.0f;
    const char* found = strstr(queryString.c_str(), "volume=");

    float logVolume = found ? (float)strtod(found + strlen("volume="), NULL) : 0;
    int i = log(logVolume);
    int linerVolume = logVolume ? 100 - int(dBConvertInverse*log(logVolume) + 0.5) : 0;
    //base::StringToDouble(std::string(found + strlen("volume=")), &volume);
    //linerVolume *= 100;
    LOG(INFO) << "AIRPLAY: got request " << uri.c_str() << " with volume " << linerVolume << "/" << logVolume;

    if (needAuth && !checkAuthorization(authorization, method, uri))
    {
      status = REQUEST_RESULT_NEED_AUTH;
    }
    else if (linerVolume >= 0 && linerVolume <= 100)
    {
#if 1
      DictionaryValue volume_dict;
      std::string eventArg;
      volume_dict.Set("instanceID", new base::FundamentalValue(0));
      volume_dict.Set("Channel", new StringValue(""));
      volume_dict.Set("Volume", new base::FundamentalValue(logVolume));
      base::JSONWriter::Write(&volume_dict, &eventArg);
      if (delegate_){
        delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_SETVOLUME, eventArg);
      }
#else
      float oldVolume = g_application.GetVolume();
      volume *= 100;
      if(oldVolume != volume && CSettings::Get().GetBool("services.airplayvolumecontrol"))
      {
        backupVolume();
        g_application.SetVolume(volume);
        CApplicationMessenger::Get().ShowVolumeBar(oldVolume < volume);
      }
#endif
    }
  }
  // Contains a header like format in the request body which should contain a
  // Content-Location and optionally a Start-Position
  else if (uri == "/play") {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    if (needAuth && !checkAuthorization(authorization, method, uri)) {
      status = REQUEST_RESULT_NEED_AUTH;
    } else {
      ProcessPlayRequest(responseHeader, responseBody);
    }
  }
  // Used to perform seeking (POST request) and to retrieve current player position (GET request).
  // GET scrub seems to also set rate 1 - strange but true
  else if (uri == "/scrub")
  {
    if (needAuth && !checkAuthorization(authorization, method, uri))
    {
      status = REQUEST_RESULT_NEED_AUTH;
    }
    else if (method == "GET")
    {
      LOG(INFO) << "AIRPLAY: got GET request " << uri;
      double duration=0, position=0;
      DictionaryValue scrub_dict;
      scrub_dict.Set("InstanceID", new base::FundamentalValue(0));
      std::string eventArg;
      std::string out;
      base::JSONWriter::Write(&scrub_dict, &eventArg);
      if (delegate_) {
        delegate_->onEventSync(CAirPlayServer::Delegate::EVENT_DMR_GETPOSITIONINFO, eventArg, &out);

        scoped_ptr<base::Value> out_dict(base::JSONReader::Read(out));
        LOG_IF(WARNING, !(out_dict->IsType(Value::TYPE_DICTIONARY))) << "Got a wrong answers.";
        DictionaryValue* dict = static_cast<DictionaryValue*>(out_dict.get());

        dict->GetDouble("Duration", &duration);
        dict->GetDouble("Position", &position);
      }
      //      responseBody.Format("duration: %.6Lf\r\nposition: %.6f\r\n", duration, position);
      base::StringAppendF(&responseBody,"duration: %.6Lf\r\nposition: %.6f\r\n", duration, position);
    }
    else
    {
      const char* found = strstr(queryString.c_str(), "position=");
      DictionaryValue scrub_dict;
      scrub_dict.Set("InstanceID", new base::FundamentalValue(0));

      double position = (int64_t) (atof(found + strlen("position=")) * 1000.0);
      scrub_dict.Set("seek_mode", new StringValue("TRACK_NR"));
      scrub_dict.Set("seek_target", new base::FundamentalValue(position));

      std::string eventArg;
      base::JSONWriter::Write(&scrub_dict, &eventArg);
      if (delegate_){
        delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_SEEK, eventArg);
      }
    }
  }

  // Sent when media playback should be stopped
  else if (uri == "/stop")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    if (needAuth && !checkAuthorization(authorization, method, uri))
    {
      status = REQUEST_RESULT_NEED_AUTH;
    }
    else
    {
      DictionaryValue scrub_dict;
      scrub_dict.Set("InstanceID", new base::FundamentalValue(0));
      std::string eventArg;
      base::JSONWriter::Write(&scrub_dict, &eventArg);
      if (delegate_)
        delegate_->onEvent(CAirPlayServer::Delegate::EVENT_DMR_STOP, eventArg);
      if (slide_show_timer_.IsRunning()) {
        slide_show_timer_.Stop();
      }
    }
  }

  // RAW JPEG data is contained in the request body
  else if (uri == "/photo")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    if (needAuth && !checkAuthorization(authorization, method, uri))
    {
      status = REQUEST_RESULT_NEED_AUTH;
    } else {
      ProcessPhotoRequest(responseHeader, responseBody);
    }
  }
  else if (uri == "/slideshows/1"){
    LOG(INFO) << "AIRPLAY: got request " << uri;
    if (needAuth && !checkAuthorization(authorization, method, uri)) {
      status = REQUEST_RESULT_NEED_AUTH;
    } else {
      ProcessSlideShowRequest(responseHeader, responseBody);
    }

  }
  else if (uri == "/playback-info")
  {
    float position = 0.0f;
    float duration = 0.0f;
    float cachePosition = 0.0f;
    bool playing = false;

    LOG(INFO) << "AIRPLAY: got request " << uri;

    if (needAuth && !checkAuthorization(authorization, method, uri))
    {
      status = REQUEST_RESULT_NEED_AUTH;
    }
    else if (1) { //added by teddy: we should check is it playing right now.
      double duration=0.0f, position=0.0f, start=0.0f, cachePosition=0.0f;
      int rate = 1;
      DictionaryValue scrub_dict;
      scrub_dict.Set("InstanceID", new base::FundamentalValue(0));
      std::string eventArg;
      std::string out;
      base::JSONWriter::Write(&scrub_dict, &eventArg);
      if (delegate_) {
        delegate_->onEventSync(CAirPlayServer::Delegate::EVENT_DMR_GETPOSITIONINFO, eventArg, &out);

        scoped_ptr<base::Value> out_dict(base::JSONReader::Read(out));
        LOG_IF(WARNING, !(out_dict->IsType(Value::TYPE_DICTIONARY))) << "Got a wrong answers.";
        DictionaryValue* dict = static_cast<DictionaryValue*>(out_dict.get());

        dict->GetDouble("Duration", &duration);
        dict->GetDouble("Position", &position);
        dict->GetDouble("Start", &start);
        dict->GetInteger("Rate", &rate);

        if (!dict->GetDouble("cachePosition", &cachePosition))
          cachePosition = position;
      }
      //responseBody.Format("duration: %.6Lf\r\nposition: %.6f\r\n", duration, position);
      //base::StringAppendF(&responseBody, "duration: %.6Lf\r\nposition: %.6f\r\n", duration, position);
      base::StringAppendF(&responseBody, kPlaybackInfo, duration, position, rate, duration, duration);
      responseHeader = "Content-Type: text/x-apple-plist+xml\r\n";
    } else {
      //responseBody.Format(PLAYBACK_INFO_NOT_READY, duration, cachePosition, position, (playing ? 1 : 0), duration);
      base::StringAppendF(&responseBody, kPlaybackInfoNotReady);
      responseHeader = "Content-Type: text/x-apple-plist+xml\r\n";
    }
  }
  else if (uri == "/server-info")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    //responseBody.Format(SERVER_INFO, CApplication::getInstance().getNetwork().GetFirstConnectedInterface()->GetMacAddress());
    base::StringAppendF(&responseBody, kServerInfo, CApplication::getInstance().getNetwork().GetFirstConnectedInterface()->GetMacAddress().c_str());
    responseHeader = "Content-Type: text/x-apple-plist+xml\r\n";
  }
  else if (uri == "/slideshow-features")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    base::StringAppendF(&responseBody, kSlideFeatures);
    responseHeader = "Content-Type: text/x-apple-plist+xml\r\n";
  }

  else if (uri == "/authorize")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    // DRM, ignore for now.
  }

  else if (uri == "/setProperty")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
#if 1
    if (contentType == "application/x-apple-binary-plist") {

      const char* bodyChr = http_parser_->getBody();

      plist_t dict = NULL;
      plist_from_bin(bodyChr, http_parser_->getContentLength(), &dict);
      {
        PList::Dictionary param_dict(dict);
        PList::Dictionary::iterator iter = param_dict.Begin();
        PList::Node* node = NULL;
        std::string sub_key;

        //param_dict's responsible for destory the plist_t, so wo forget it.
        dict = NULL;

        while( iter != param_dict.End()){
          node = iter->second;
          sub_key = iter->first;
          switch( node->GetType()){
          case PLIST_BOOLEAN:
            LOG(INFO) << "PLIST_BOOLEAN key=" << iter->first << " value=" << static_cast<PList::Boolean*>(node)->GetValue();
            break;
          case PLIST_UINT:
            LOG(INFO) << "PLIST_UINT key=" << iter->first << " value=" << static_cast<PList::Integer*>(node)->GetValue();
            break;
          case PLIST_REAL:
            LOG(INFO) << "PLIST_REAL key=" << iter->first << " value=" << static_cast<PList::Real*>(node)->GetValue();
            break;
          case PLIST_STRING:
            LOG(INFO) << "PLIST_STRING key=" << iter->first << " value=" << static_cast<PList::String*>(node)->GetValue();
            break;
          case PLIST_KEY:
            break;
          case PLIST_DATA:
            break;
          case PLIST_DATE:
            {
              timeval tv = static_cast<PList::Date*>(node)->GetValue();
              LOG(INFO) << "PLIST_DATE key=" << iter->first << " time.tv_sec=" << tv.tv_sec << " time.tv_usec=" << tv.tv_usec;
            }
            break;
          case PLIST_ARRAY:
            break;
          case PLIST_DICT:
            break;
          case PLIST_NONE:
          default:
            break;
          }
          iter ++;
        }
      }
      if (dict) {
        plist_free(dict);
      }
    }
#endif
    status = REQUEST_RESULT_NOT_FOUND;
  }
  else if (uri == "/getProperty")
  {
    LOG(INFO) << "AIRPLAY: got request " << uri;
    status = REQUEST_RESULT_NOT_FOUND;
  }
  else if (uri == "200" || uri == "204") //response OK from the event reverse message
  {
    ProcessReverseResponse(responseHeader, responseBody);
    LOG(INFO) << "AIRPLAY: got request " << uri;
    status = REQUEST_RESULT_NO_RESPONSE_NEEDED;
  }
  else
  {
    LOG(INFO) << "AIRPLAY Server: unhandled request " << uri;
    status = REQUEST_RESULT_NOT_IMPLEMENTED;
  }

  if (status == REQUEST_RESULT_NEED_AUTH)
  {
    ComposeAuthRequestAnswer(responseHeader, responseBody);
  }

  return status;
}

#endif

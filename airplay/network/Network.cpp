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
#include "base/logging.h"


#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "Network.h"
//#include "ApplicationMessenger.h"
#include "network/NetworkServices.h"

#ifdef TARGET_WINDOWS
//#include "utils/SystemInfo.h"
#endif

using namespace std;

/* slightly modified in_ether taken from the etherboot project (http://sourceforge.net/projects/etherboot) */
bool in_ether (const char *bufp, unsigned char *addr)
{
  if (strlen(bufp) != 17)
    return false;

  char c;
  const char *orig;
  unsigned char *ptr = addr;
  unsigned val;

  int i = 0;
  orig = bufp;

  while ((*bufp != '\0') && (i < 6))
  {
    val = 0;
    c = *bufp++;

    if (isdigit(c))
      val = c - '0';
    else if (c >= 'a' && c <= 'f')
      val = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      val = c - 'A' + 10;
    else
      return false;

    val <<= 4;
    c = *bufp;
    if (isdigit(c))
      val |= c - '0';
    else if (c >= 'a' && c <= 'f')
      val |= c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      val |= c - 'A' + 10;
    else if (c == ':' || c == '-' || c == 0)
      val >>= 4;
    else
      return false;

    if (c != 0)
      bufp++;

    *ptr++ = (unsigned char) (val & 0377);
    i++;

    if (*bufp == ':' || *bufp == '-')
      bufp++;
  }

  if (bufp - orig != 17)
    return false;

  return true;
}

int NetworkAccessPoint::getQuality() const
{
  // Cisco dBm lookup table (partially nonlinear)
  // Source: "Converting Signal Strength Percentage to dBm Values, 2002"
  int quality;
  if (m_dBm >= -10) quality = 100;
  else if (m_dBm >= -20) quality = 85 + (m_dBm + 20);
  else if (m_dBm >= -30) quality = 77 + (m_dBm + 30);
  else if (m_dBm >= -60) quality = 48 + (m_dBm + 60);
  else if (m_dBm >= -98) quality = 13 + (m_dBm + 98);
  else if (m_dBm >= -112) quality = 1 + (m_dBm + 112);
  else quality = 0;
  return quality;
}

int NetworkAccessPoint::FreqToChannel(float frequency)
{
  int IEEE80211Freq[] = {2412, 2417, 2422, 2427, 2432,
                         2437, 2442, 2447, 2452, 2457,
                         2462, 2467, 2472, 2484,
                         5180, 5200, 5210, 5220, 5240, 5250,
                         5260, 5280, 5290, 5300, 5320,
                         5745, 5760, 5765, 5785, 5800, 5805, 5825};
  int IEEE80211Ch[] =   {   1,    2,    3,    4,    5,
                            6,    7,    8,    9,   10,
                           11,   12,   13,   14,
                           36,   40,   42,   44,   48,   50,
                           52,   56,   58,   60,   64,
                          149,  152,  153,  157,  160,  161,  165};
  // Round frequency to the nearest MHz
  int mod_chan = (int)(frequency / 1000000 + 0.5f);
  for (unsigned int i = 0; i < sizeof(IEEE80211Freq) / sizeof(int); ++i)
  {
    if (IEEE80211Freq[i] == mod_chan)
      return IEEE80211Ch[i];
  }
  return 0; // unknown
}


CNetwork::CNetwork()
{
  //CApplicationMessenger::Get().NetworkMessage(SERVICES_UP, 0);
}

CNetwork::~CNetwork()
{
  //CApplicationMessenger::Get().NetworkMessage(SERVICES_DOWN, 0);
}

int CNetwork::ParseHex(char *str, unsigned char *addr)
{
   int len = 0;

   while (*str)
   {
      int tmp;
      if (str[1] == 0)
         return -1;
      if (sscanf(str, "%02x", (unsigned int *)&tmp) != 1)
         return -1;
      addr[len] = tmp;
      len++;
      str += 2;
   }

   return len;
}

std::string CNetwork::GetHostName(void)
{
  char hostName[128];
  if (gethostname(hostName, sizeof(hostName)))
    return std::string("unknown");
  else
    return std::string(hostName);
}

CNetworkInterface* CNetwork::GetFirstConnectedInterface()
{
   vector<CNetworkInterface*>& ifaces = GetInterfaceList();
   vector<CNetworkInterface*>::const_iterator iter = ifaces.begin();
   while (iter != ifaces.end())
   {
      CNetworkInterface* iface = *iter;
      if (iface && iface->IsConnected())
         return iface;
      ++iter;
   }

   return NULL;
}

bool CNetwork::HasInterfaceForIP(unsigned long address)
{
   unsigned long subnet;
   unsigned long local;
   vector<CNetworkInterface*>& ifaces = GetInterfaceList();
   vector<CNetworkInterface*>::const_iterator iter = ifaces.begin();
   while (iter != ifaces.end()) {
      CNetworkInterface* iface = *iter;
      if (iface && iface->IsConnected()) {
        subnet = ntohl(inet_addr(iface->GetCurrentNetmask().c_str()));
        local = ntohl(inet_addr(iface->GetCurrentIPAddress().c_str()));
        if( (address & subnet) == (local & subnet) )
          return true;
      }
      ++iter;
   }

   return false;
}

bool CNetwork::IsAvailable(bool wait /*= false*/)
{
  if (wait)
  {
    // NOTE: Not implemented in linuxport branch as 99.9% of the time
    //       we have the network setup already.  Trunk code has a busy
    //       wait for 5 seconds here.
  }

  vector<CNetworkInterface*>& ifaces = GetInterfaceList();
  return (ifaces.size() != 0);
}

bool CNetwork::IsConnected()
{
   return GetFirstConnectedInterface() != NULL;
}

CNetworkInterface* CNetwork::GetInterfaceByName(std::string& name)
{
   vector<CNetworkInterface*>& ifaces = GetInterfaceList();
   vector<CNetworkInterface*>::const_iterator iter = ifaces.begin();
   while (iter != ifaces.end()) {
      CNetworkInterface* iface = *iter;
      //if (iface && iface->GetName().Equals(name))
      if (iface && iface->GetName() == name)
         return iface;
      ++iter;
   }

   return NULL;
}

void CNetwork::NetworkMessage(EMESSAGE message, int param)
{
  switch( message )
  {
    case SERVICES_UP:
      LOG(INFO) << "Starting network services";
      CNetworkServices::Get().Start();
      break;

    case SERVICES_DOWN:
      LOG(INFO) << "Signaling network services to stop";
      CNetworkServices::Get().Stop(false); // tell network services to stop, but don't wait for them yet
      LOG(INFO) << "Waiting for network services to stop";
      CNetworkServices::Get().Stop(true); // wait for network services to stop
      break;
  }
}

bool CNetwork::WakeOnLan(const char* mac)
{
  int i, j, packet;
  unsigned char ethaddr[8];
  unsigned char buf [128];
  unsigned char *ptr;

  // Fetch the hardware address
  if (!in_ether(mac, ethaddr))
  {
    LOG(ERROR_REPORT) << "Invalid hardware address specified (%s)";
    return false;
  }

  // Setup the socket
  if ((packet = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
  {
    LOG(ERROR_REPORT) << "Unable to create socket (%s)" << strerror (errno);
    return false;
  }

  // Set socket options
  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  saddr.sin_port = htons(9);

  unsigned int value = 1;
  if (setsockopt (packet, SOL_SOCKET, SO_BROADCAST, (char*) &value, sizeof( unsigned int ) ) == SOCKET_ERROR)
  {
    LOG(ERROR_REPORT) << "Unable to set socket options (%s)" << strerror (errno);
    closesocket(packet);
    return false;
  }

  // Build the magic packet (6 x 0xff + 16 x MAC address)
  ptr = buf;
  for (i = 0; i < 6; i++)
    *ptr++ = 0xff;

  for (j = 0; j < 16; j++)
    for (i = 0; i < 6; i++)
      *ptr++ = ethaddr[i];

  // Send the magic packet
  if (sendto (packet, (char *)buf, 102, 0, (struct sockaddr *)&saddr, sizeof (saddr)) < 0)
  {
    LOG(ERROR_REPORT) << "Unable to send magic packet (%s)" << strerror (errno);
    closesocket(packet);
    return false;
  }

  closesocket(packet);
  LOG(INFO) << "Magic packet send to " << mac;
  return true;
}

// ping helper
static const char* ConnectHostPort(SOCKET soc, const struct sockaddr_in& addr, struct timeval& timeOut, bool tryRead)
{
  // set non-blocking
#ifdef TARGET_WINDOWS
  u_long nonblocking = 1;
  int result = ioctlsocket(soc, FIONBIO, &nonblocking);
#else
  int result = fcntl(soc, F_SETFL, fcntl(soc, F_GETFL) | O_NONBLOCK);
#endif

  if (result != 0)
    return "set non-blocking option failed";

  result = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); // non-blocking connect, will fail ..

  if (result < 0)
  {
#ifdef TARGET_WINDOWS
    if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
    if (errno != EINPROGRESS)
#endif
      return "unexpected connect fail";

    { // wait for connect to complete
      fd_set wset;
      FD_ZERO(&wset);
      FD_SET(soc, &wset);

      result = select(FD_SETSIZE, 0, &wset, 0, &timeOut);
    }

    if (result < 0)
      return "select fail";

    if (result == 0) // timeout
      return ""; // no error

    { // verify socket connection state
      int err_code = -1;
      socklen_t code_len = sizeof (err_code);

      result = getsockopt(soc, SOL_SOCKET, SO_ERROR, (char*) &err_code, &code_len);

      if (result != 0)
        return "getsockopt fail";

      if (err_code != 0)
        return ""; // no error, just not connected
    }
  }

  if (tryRead)
  {
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(soc, &rset);

    result = select(FD_SETSIZE, &rset, 0, 0, &timeOut);

    if (result > 0)
    {
      char message [32];

      result = recv(soc, message, sizeof(message), 0);
    }

    if (result == 0)
      return ""; // no reply yet

    if (result < 0)
      return "recv fail";
  }

  return 0; // success
}

bool CNetwork::PingHost(unsigned long ipaddr, unsigned short port, unsigned int timeOutMs, bool readability_check)
{
  if (port == 0) // use icmp ping
    return PingHost (ipaddr, timeOutMs);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ipaddr;

  SOCKET soc = socket(AF_INET, SOCK_STREAM, 0);

  const char* err_msg = "invalid socket";

  if (soc != INVALID_SOCKET)
  {
    struct timeval tmout;
    tmout.tv_sec = timeOutMs / 1000;
    tmout.tv_usec = (timeOutMs % 1000) * 1000;

    err_msg = ConnectHostPort (soc, addr, tmout, readability_check);

    (void) closesocket (soc);
  }

  if (err_msg && *err_msg)
  {
#ifdef TARGET_WINDOWS
    //std::string sock_err = WUSysMsg(WSAGetLastError());
    LOG_GETLASTERROR_STREAM(ERROR_REPORT) << " Ping Host (" << inet_ntoa(addr.sin_addr) << ":" << port
        << ")";
#else
    //std::string sock_err = strerror(errno);
    LOG_ERRNO_STREAM(ERROR_REPORT) << " Ping Host (" << inet_ntoa(addr.sin_addr) << ":" << port
        << ")";
#endif
    //LOG(ERROR_REPORT) << "Ping Host (%s)" << inet_ntoa(addr.sin_addr) << ":" << port
    //  << ") - " << err_msg << "(" << sock_err.c_str() << ")";
  }

  return err_msg == 0;
}

//creates, binds and listens a tcp socket on the desired port. Set bindLocal to
//true to bind to localhost only. The socket will listen over ipv6 if possible
//and fall back to ipv4 if ipv6 is not available on the platform.
int CreateTCPServerSocket(const int port, const bool bindLocal, const int backlog, const char *callerName)
{
  struct sockaddr_storage addr;
  struct sockaddr_in6 *s6;
  struct sockaddr_in  *s4;
  int    sock;
  bool   v4_fallback = false;
#ifdef TARGET_WINDOWS
  // Windows XP and earlier don't support the IPV6_V6ONLY socket option
  // so always fall back to IPv4 directly to keep old functionality
//  if (CSysInfo::GetWindowsVersion() <= CSysInfo::WindowsVersionWinXP)
    v4_fallback = true;
#endif

#ifdef WINSOCK_VERSION
  int yes = 1;
  int no = 0;
#else
  unsigned int yes = 1;
  unsigned int no = 0;
#endif

  memset(&addr, 0, sizeof(addr));

  if (!v4_fallback &&
     (sock = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP)) >= 0)
  {
    // in case we're on ipv6, make sure the socket is dual stacked
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&no, sizeof(no)) < 0)
    {
#ifdef _MSC_VER
      //std::string sock_err = WUSysMsg(WSAGetLastError());
      LOG_GETLASTERROR_STREAM(WARNING) << " Server: Only IPv6 supported.";
#else
      //std::string sock_err = strerror(errno);
      LOG_ERRNO_STREAM(WARNING) << " Server: Only IPv6 supported.";
#endif
      //LOG(WARNING) << callerName << " Server: Only IPv6 supported: " << sock_err.c_str();
    }
  }
  else
  {
    v4_fallback = true;
    sock = socket(PF_INET, SOCK_STREAM, 0);
  }

  if (sock == INVALID_SOCKET)
  {
    LOG(ERROR_REPORT) << callerName << " Server: Failed to create serversocket";
    return INVALID_SOCKET;
  }

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

  if (v4_fallback)
  {
    addr.ss_family = AF_INET;
    s4 = (struct sockaddr_in *) &addr;
    s4->sin_port = htons(port);

    if (bindLocal)
      s4->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    else
      s4->sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else
  {
    addr.ss_family = AF_INET6;
    s6 = (struct sockaddr_in6 *) &addr;
    s6->sin6_port = htons(port);

    if (bindLocal)
      s6->sin6_addr = in6addr_loopback;
    else
      s6->sin6_addr = in6addr_any;
  }

  if (::bind( sock, (struct sockaddr *) &addr,
            (addr.ss_family == AF_INET6) ? sizeof(struct sockaddr_in6) :
                                           sizeof(struct sockaddr_in)  ) < 0)
  {
    closesocket(sock);
    LOG(ERROR_REPORT) << callerName << " Server: Failed to bind serversocket";
    return INVALID_SOCKET;
  }

  if (listen(sock, backlog) < 0)
  {
    closesocket(sock);
    LOG(ERROR_REPORT) << callerName << " Server: Failed to set listen", callerName;
    return INVALID_SOCKET;
  }

  return sock;
}


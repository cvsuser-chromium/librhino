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
#include "ZeroconfMDNS.h"

#include "system.h"

#include "base/time/time.h"
#include "base/threading/platform_thread.h"

#include <string>
#include <sstream>
#include <sys/time.h>

#include <threads/SingleLock.h>
#include <utils/log.h>

#if defined(HAS_MDNS_EMBEDDED)
#include <mDnsEmbedded.h>
#endif //HAS_MDNS_EMBEDDED

//#pragma comment(lib, "dnssd.lib")

void CZeroconfMDNS::Process()
{
#if defined(HAS_MDNS_EMBEDDED)
  LOG(INFO) << "ZeroconfEmbedded - processing...";
  struct timeval timeout;
  timeout.tv_sec = 1;
  while (( !m_bStop ))
    embedded_mDNSmainLoop(timeout);
#endif //HAS_MDNS_EMBEDDED
}


CZeroconfMDNS::CZeroconfMDNS()  : CThread("ZerocconfEmbedded")
{
  m_service = NULL;
#if defined(HAS_MDNS_EMBEDDED)
  embedded_mDNSInit();
  Create();
#endif //HAS_MDNS_EMBEDDED
}

CZeroconfMDNS::~CZeroconfMDNS()
{
  doStop();
#if defined(HAS_MDNS_EMBEDDED)
  StopThread();
  embedded_mDNSExit();
#endif //HAS_MDNS_EMBEDDED
}

bool CZeroconfMDNS::IsZCdaemonRunning()
{
  return true;
}

//methods to implement for concrete implementations
bool CZeroconfMDNS::doPublishService(const std::string& fcr_identifier,
                      const std::string& fcr_type,
                      const std::string& fcr_name,
                      unsigned int f_port,
                      const std::vector<std::pair<std::string, std::string> >& txt)
{
  DNSServiceRef netService = NULL;
  TXTRecordRef txtRecord;
  DNSServiceErrorType err = kDNSServiceErr_Unknown;
  TXTRecordCreate(&txtRecord, 0, NULL);

  LOG(INFO) << "ZeroconfMDNS: identifier: " << fcr_identifier.c_str()
                               << " type: " << fcr_type.c_str()
                               << " name: " << fcr_name.c_str()
                               << " port: " << f_port;

  //add txt records
  if(!txt.empty())
  {
    for(std::vector<std::pair<std::string, std::string> >::const_iterator it = txt.begin(); it != txt.end(); ++it)
    {
      LOG(INFO) << "ZeroconfMDNS: key: " << it->first.c_str()
                           << " value: " << it->second.c_str();
      uint8_t txtLen = (uint8_t)strlen(it->second.c_str());
      TXTRecordSetValue(&txtRecord, it->first.c_str(), txtLen, it->second.c_str());
    }
  }

  {
    CSingleLock lock(m_data_guard);
    netService = m_service;
    err = DNSServiceRegister(&netService, kDNSServiceFlagsShareConnection, 0,
                              fcr_name.c_str(), fcr_type.c_str(), NULL, NULL, htons(f_port),
                              TXTRecordGetLength(&txtRecord),
                              TXTRecordGetBytesPtr(&txtRecord),
                              registerCallback, NULL);
  }

  if (err != kDNSServiceErr_NoError) {
    // Something went wrong so lets clean up.
    if (netService) {
      DNSServiceRefDeallocate(netService);
    }

    LOG(ERROR_REPORT) << "ZeroconfMDNS: DNSServiceRegister returned error = "<< err;
  } else {
    CSingleLock lock(m_data_guard);
    m_services.insert(make_pair(fcr_identifier, netService));
  }

  TXTRecordDeallocate(&txtRecord);

  return err == kDNSServiceErr_NoError;
}

bool CZeroconfMDNS::doRemoveService(const std::string& fcr_ident)
{
  CSingleLock lock(m_data_guard);
  tServiceMap::iterator it = m_services.find(fcr_ident);
  if(it != m_services.end()) {
    DNSServiceRefDeallocate(it->second);
    m_services.erase(it);
    LOG(INFO) << "ZeroconfMDNS: Removed service %s" << fcr_ident;
    return true;
  }
  else
    return false;
}

void CZeroconfMDNS::doStop()
{
  {
    CSingleLock lock(m_data_guard);
    LOG(INFO) << "ZeroconfMDNS: Shutdown services";
    for(tServiceMap::iterator it = m_services.begin(); it != m_services.end(); ++it)
    {
      DNSServiceRefDeallocate(it->second);
      LOG(INFO) << "ZeroconfMDNS: Removed service %s" << it->first;
    }
    m_services.clear();
  }
  {
    CSingleLock lock(m_data_guard);

    if (m_service)
      DNSServiceRefDeallocate(m_service);
    m_service = NULL;
  }
}

void DNSSD_API CZeroconfMDNS::registerCallback(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode, const char *name, const char *regtype, const char *domain, void *context)
{
  (void)sdref;    // Unused
  (void)flags;    // Unused
  (void)context;  // Unused

  if (errorCode == kDNSServiceErr_NoError)
  {
    if (flags & kDNSServiceFlagsAdd){
      LOG(INFO) << "ZeroconfMDNS: [" << name << "." << regtype << "." << domain << "] now registered and active";
    } else {
      LOG(INFO) << "ZeroconfMDNS: [" << name << "." << regtype << "." << domain
                <<"] registration removed, flags=" << flags;
    }
  }
  else if (errorCode == kDNSServiceErr_NameConflict) {
     //CLog::Log(LOGDEBUG, "ZeroconfMDNS: %s.%s%s Name in use, please choose another", name, regtype, domain);
     LOG(INFO) << "ZeroconfMDNS: [" << name << "." << regtype << "." << domain <<
          "] Name in use, please choose another";
  } else {
    //CLog::Log(LOGDEBUG, "ZeroconfMDNS: %s.%s%s error code %d", name, regtype, domain, errorCode);
    LOG(INFO) << "ZeroconfMDNS: [" << name << "." << regtype << "." << domain <<
          "] errorCode=" << errorCode;
  }
}

void CZeroconfMDNS::ProcessResults()
{
  CSingleLock lock(m_data_guard);
  DNSServiceErrorType err = DNSServiceProcessResult(m_service);
  if (err != kDNSServiceErr_NoError) {
    //CLog::Log(LOGERROR, "ZeroconfMDNS: DNSServiceProcessResult returned (error = %ld)", (int) err);
    LOG(INFO) << "ZeroconfMDNS: DNSServiceProcessResult returned error = " << err;
  }
}


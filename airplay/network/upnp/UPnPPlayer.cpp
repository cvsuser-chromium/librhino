/*
 *      Copyright (c) 2006 elupus (Joakim Plate)
 *      Copyright (C) 2006-2013 Team XBMC
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

#include "UPnPPlayer.h"
#include "UPnP.h"
#include "UPnPInternal.h"
#include "Platinum.h"
#include "PltMediaController.h"
#include "PltDidl.h"
#include "FileItem.h"
#include "threads/Event.h"
#include "utils/log.h"
//#include "utils/TimeUtils.h"
//#include "GUIInfoManager.h"
//#include "ThumbLoader.h"
//#include "video/VideoThumbLoader.h"
//#include "music/MusicThumbLoader.h"
//#include "ApplicationMessenger.h"
#include "Application.h"
//#include "dialogs/GUIDialogBusy.h"
//#include "guilib/GUIWindowManager.h"


NPT_SET_LOCAL_LOGGER("xbmc.upnp.player")

namespace UPNP
{

class CUPnPPlayerController
  : public PLT_MediaControllerDelegate
{
public:
  CUPnPPlayerController(PLT_MediaController* control, PLT_DeviceDataReference& device, IPlayerCallback& callback)
    : m_control(control)
    , m_transport(NULL)
    , m_device(device)
    , m_instance(0)
    , m_callback(callback)
    , m_postime(0)
  {
    memset(&m_posinfo, 0, sizeof(m_posinfo));
    m_device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", m_transport);
  }

  virtual void OnSetAVTransportURIResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata)
  {
    if(NPT_FAILED(res))
      CLog::Log(LOGERROR, "UPNP: CUPnPPlayer : OnSetAVTransportURIResult failed");
    m_resstatus = res;
    m_resevent.Set();
  }

  virtual void OnPlayResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata)
  {
    if(NPT_FAILED(res))
      CLog::Log(LOGERROR, "UPNP: CUPnPPlayer : OnPlayResult failed");
    m_resstatus = res;
    m_resevent.Set();
  }

  virtual void OnStopResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata)
  {
    if(NPT_FAILED(res))
      CLog::Log(LOGERROR, "UPNP: CUPnPPlayer : OnStopResult failed");
    m_resstatus = res;
    m_resevent.Set();
  }

  virtual void OnGetMediaInfoResult(NPT_Result res, PLT_DeviceDataReference& device, PLT_MediaInfo* info, void* userdata)
  {
    if(NPT_FAILED(res) || info == NULL)
      CLog::Log(LOGERROR, "UPNP: CUPnPPlayer : OnGetMediaInfoResult failed");
  }

  virtual void OnGetTransportInfoResult(NPT_Result res, PLT_DeviceDataReference& device, PLT_TransportInfo* info, void* userdata)
  {
    CSingleLock lock(m_section);

    if(NPT_FAILED(res))
    {
      CLog::Log(LOGERROR, "UPNP: CUPnPPlayer : OnGetTransportInfoResult failed");
      m_trainfo.cur_speed            = "0";
      m_trainfo.cur_transport_state  = "STOPPED";
      m_trainfo.cur_transport_status = "ERROR_OCCURED";
    }
    else
      m_trainfo = *info;
    m_traevnt.Set();
  }

  void UpdatePositionInfo()
  {
#if 1
#else
    if(m_postime == 0
    || m_postime > CTimeUtils::GetFrameTime())
      return;
#endif
    m_control->GetTransportInfo(m_device, m_instance, this);
    m_control->GetPositionInfo(m_device, m_instance, this);

    m_postime = 0;
  }

  virtual void OnGetPositionInfoResult(NPT_Result res, PLT_DeviceDataReference& device, PLT_PositionInfo* info, void* userdata)
  {
    CSingleLock lock(m_section);

    if(NPT_FAILED(res) || info == NULL)
    {
      CLog::Log(LOGERROR, "UPNP: CUPnPPlayer : OnGetMediaInfoResult failed");
      m_posinfo = PLT_PositionInfo();
    }
    else
      m_posinfo = *info;
#if 1
#else
    m_postime = CTimeUtils::GetFrameTime() + 500;
#endif
    m_posevnt.Set();
  }


  ~CUPnPPlayerController()
  {
  }

  PLT_MediaController*     m_control;
  PLT_Service *            m_transport;
  PLT_DeviceDataReference  m_device;
  NPT_UInt32               m_instance;
  IPlayerCallback&         m_callback;

  NPT_Result               m_resstatus;
  CEvent                   m_resevent;

  CCriticalSection         m_section;
  unsigned int             m_postime;

  CEvent                   m_posevnt;
  PLT_PositionInfo         m_posinfo;

  CEvent                   m_traevnt;
  PLT_TransportInfo        m_trainfo;
};

CUPnPPlayer::CUPnPPlayer(IPlayerCallback& callback, const char* uuid)
: IPlayer(callback)
, m_control(NULL)
, m_delegate(NULL)
, m_started(false)
{
  m_control  = CUPnP::GetInstance()->m_MediaController;

  PLT_DeviceDataReference device;
  if(NPT_SUCCEEDED(m_control->FindRenderer(uuid, device)))
    m_delegate = new CUPnPPlayerController(m_control, device, callback);
  else
    CLog::Log(LOGERROR, "UPNP: CUPnPPlayer couldn't find device as %s", uuid);
}

CUPnPPlayer::~CUPnPPlayer()
{
  CloseFile();
  delete m_delegate;
}

static NPT_Result WaitOnEvent(CEvent& event, XbmcThreads::EndTime& timeout, CGUIDialogBusy*& dialog)
{
  if(event.WaitMSec(0))
    return NPT_SUCCESS;

  if(dialog == NULL) {
    dialog = (CGUIDialogBusy*)g_windowManager.GetWindow(WINDOW_DIALOG_BUSY);
    dialog->Show();
  }

  g_windowManager.ProcessRenderLoop(false);

  do {
    if(event.WaitMSec(100))
      return NPT_SUCCESS;

    g_windowManager.ProcessRenderLoop(false);

    if(dialog->IsCanceled())
      return NPT_FAILURE;

  } while(!timeout.IsTimePast());

  return NPT_FAILURE;
}

bool CUPnPPlayer::OpenFile(const CFileItem& file, const CPlayerOptions& options)
{
  CFileItem item(file);
  NPT_Reference<CThumbLoader> thumb_loader;
  NPT_Reference<PLT_MediaObject> obj;
  NPT_String path(file.GetPath().c_str());
  NPT_String tmp, resource;
  XbmcThreads::EndTime timeout;
  CGUIDialogBusy* dialog = NULL;

  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);

  timeout.Set(10000);

  /* if no path we want to attach to a already playing player */
  if(path != "") {
    if (file.IsVideoDb())
      thumb_loader = NPT_Reference<CThumbLoader>(new CVideoThumbLoader());
    else if (item.IsMusicDb())
      thumb_loader = NPT_Reference<CThumbLoader>(new CMusicThumbLoader());

    obj = BuildObject(item, path, false, thumb_loader, NULL, CUPnP::GetServer());
    if(obj.IsNull()) goto failed;

    NPT_CHECK_LABEL_SEVERE(PLT_Didl::ToDidl(*obj, "", tmp), failed);
    tmp.Insert(didl_header, 0);
    tmp.Append(didl_footer);

    /* The resource uri's are stored in the Didl. We must choose the best resource
     * for the playback device */
    NPT_Cardinal res_index;
    NPT_CHECK_LABEL_SEVERE(m_control->FindBestResource(m_delegate->m_device, *obj, res_index), failed);


    /* dlna specifies that a return code of 705 should be returned
     * if TRANSPORT_STATE is not STOPPED or NO_MEDIA_PRESENT */
    NPT_CHECK_LABEL_SEVERE(m_control->Stop(m_delegate->m_device
                                           , m_delegate->m_instance
                                           , m_delegate), failed);
    NPT_CHECK_LABEL_SEVERE(WaitOnEvent(m_delegate->m_resevent, timeout, dialog), failed);
    NPT_CHECK_LABEL_SEVERE(m_delegate->m_resstatus, failed);


    NPT_CHECK_LABEL_SEVERE(m_control->SetAVTransportURI(m_delegate->m_device
                                                      , m_delegate->m_instance
                                                      , obj->m_Resources[res_index].m_Uri
                                                      , (const char*)tmp
                                                      , m_delegate), failed);
    NPT_CHECK_LABEL_SEVERE(WaitOnEvent(m_delegate->m_resevent, timeout, dialog), failed);
    NPT_CHECK_LABEL_SEVERE(m_delegate->m_resstatus, failed);

    NPT_CHECK_LABEL_SEVERE(m_control->Play(m_delegate->m_device
                                         , m_delegate->m_instance
                                         , "1"
                                         , m_delegate), failed);
    NPT_CHECK_LABEL_SEVERE(WaitOnEvent(m_delegate->m_resevent, timeout, dialog), failed);
    NPT_CHECK_LABEL_SEVERE(m_delegate->m_resstatus, failed);
  }


  /* wait for PLAYING state */
  do {
    NPT_CHECK_LABEL_SEVERE(m_control->GetTransportInfo(m_delegate->m_device
                                                     , m_delegate->m_instance
                                                     , m_delegate), failed);


    { CSingleLock lock(m_delegate->m_section);
      if(m_delegate->m_trainfo.cur_transport_state == "PLAYING"
      || m_delegate->m_trainfo.cur_transport_state == "PAUSED_PLAYBACK")
        break;

      if(m_delegate->m_trainfo.cur_transport_state  == "STOPPED"
      && m_delegate->m_trainfo.cur_transport_status != "OK")
      {
        CLog::Log(LOGERROR, "UPNP: CUPnPPlayer::OpenFile - remote player signalled error %s", file.GetPath().c_str());
        goto failed;
      }
    }

    NPT_CHECK_LABEL_SEVERE(WaitOnEvent(m_delegate->m_traevnt, timeout, dialog), failed);

  } while(!timeout.IsTimePast());

  if(options.starttime > 0)
  {
    /* many upnp units won't load file properly until after play (including xbmc) */
    NPT_CHECK_LABEL(m_control->Seek(m_delegate->m_device
                                    , m_delegate->m_instance
                                    , "REL_TIME"
                                    , PLT_Didl::FormatTimeStamp((NPT_UInt32)options.starttime)
                                    , m_delegate), failed);
  }

  m_started = true;
  m_callback.OnPlayBackStarted();
  NPT_CHECK_LABEL_SEVERE(m_control->GetPositionInfo(m_delegate->m_device
                                                  , m_delegate->m_instance
                                                  , m_delegate), failed);
  NPT_CHECK_LABEL_SEVERE(m_control->GetMediaInfo(m_delegate->m_device
                                               , m_delegate->m_instance
                                               , m_delegate), failed);

  if(dialog)
    dialog->Close();

  return true;
failed:
  CLog::Log(LOGERROR, "UPNP: CUPnPPlayer::OpenFile - unable to open file %s", file.GetPath().c_str());
  if(dialog)
    dialog->Close();
  return false;
}

bool CUPnPPlayer::QueueNextFile(const CFileItem& file)
{
  CFileItem item(file);
  NPT_Reference<CThumbLoader> thumb_loader;
  NPT_Reference<PLT_MediaObject> obj;
  NPT_String path(file.GetPath().c_str());
  NPT_String tmp;

  if (file.IsVideoDb())
    thumb_loader = NPT_Reference<CThumbLoader>(new CVideoThumbLoader());
  else if (item.IsMusicDb())
    thumb_loader = NPT_Reference<CThumbLoader>(new CMusicThumbLoader());


  obj = BuildObject(item, path, 0, thumb_loader, NULL, CUPnP::GetServer());
  if(!obj.IsNull())
  {
    NPT_CHECK_LABEL_SEVERE(PLT_Didl::ToDidl(*obj, "", tmp), failed);
    tmp.Insert(didl_header, 0);
    tmp.Append(didl_footer);
  }

  NPT_CHECK_LABEL_WARNING(m_control->SetNextAVTransportURI(m_delegate->m_device
                                                         , m_delegate->m_instance
                                                         , file.GetPath().c_str()
                                                         , (const char*)tmp
                                                         , m_delegate), failed);
  if(!m_delegate->m_resevent.WaitMSec(10000)) goto failed;
  NPT_CHECK_LABEL_WARNING(m_delegate->m_resstatus, failed);
  return true;

failed:
  CLog::Log(LOGERROR, "UPNP: CUPnPPlayer::QueueNextFile - unable to queue file %s", file.GetPath().c_str());
  return false;
}

bool CUPnPPlayer::CloseFile()
{
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  NPT_CHECK_LABEL(m_control->Stop(m_delegate->m_device
                                , m_delegate->m_instance
                                , m_delegate), failed);
  if(!m_delegate->m_resevent.WaitMSec(10000)) goto failed;
  NPT_CHECK_LABEL(m_delegate->m_resstatus, failed);
  m_callback.OnPlayBackStopped();
  return true;
failed:
  CLog::Log(LOGERROR, "UPNP: CUPnPPlayer::CloseFile - unable to stop playback");
  return false;
}

void CUPnPPlayer::Pause()
{
  if(IsPaused())
    NPT_CHECK_LABEL(m_control->Play(m_delegate->m_device
                                  , m_delegate->m_instance
                                  , "1"
                                  , m_delegate), failed);
  else
    NPT_CHECK_LABEL(m_control->Pause(m_delegate->m_device
                                   , m_delegate->m_instance
                                   , m_delegate), failed);

  return;
failed:
  CLog::Log(LOGERROR, "UPNP: CUPnPPlayer::CloseFile - unable to pause/unpause playback");
  return;
}

void CUPnPPlayer::SeekTime(__int64 ms)
{
  NPT_CHECK_LABEL(m_control->Seek(m_delegate->m_device
                                , m_delegate->m_instance
                                , "REL_TIME", PLT_Didl::FormatTimeStamp((NPT_UInt32)(ms / 1000))
                                , m_delegate), failed);

  g_infoManager.SetDisplayAfterSeek();
  return;
failed:
  CLog::Log(LOGERROR, "UPNP: CUPnPPlayer::SeekTime - unable to seek playback");
}

float CUPnPPlayer::GetPercentage()
{
  int64_t tot = GetTotalTime();
  if(tot)
    return 100.0f * GetTime() / tot;
  else
    return 0.0f;
}

void CUPnPPlayer::SeekPercentage(float percent)
{
  int64_t tot = GetTotalTime();
  if (tot)
    SeekTime((int64_t)(tot * percent / 100));
}

void CUPnPPlayer::Seek(bool bPlus, bool bLargeStep)
{
}

void CUPnPPlayer::DoAudioWork()
{
  NPT_String data;
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  m_delegate->UpdatePositionInfo();

  if(m_started) {
    NPT_String uri, meta;
    NPT_CHECK_LABEL(m_delegate->m_transport->GetStateVariableValue("CurrentTrackURI", uri), failed);
    NPT_CHECK_LABEL(m_delegate->m_transport->GetStateVariableValue("CurrentTrackMetadata", meta), failed);

    if(m_current_uri  != (const char*)uri
    || m_current_meta != (const char*)meta) {
      m_current_uri  = (const char*)uri;
      m_current_meta = (const char*)meta;
      CFileItemPtr item = GetFileItem(uri, meta);
      g_application.CurrentFileItem() = *item;
      CApplicationMessenger::Get().SetCurrentItem(*item.get());
    }

    NPT_CHECK_LABEL(m_delegate->m_transport->GetStateVariableValue("TransportState", data), failed);
    if(data == "STOPPED")
    {
      m_started = false;
      m_callback.OnPlayBackEnded();
    }
  }
  return;
failed:
  return;
}

bool CUPnPPlayer::IsPlaying() const
{
  NPT_String data;
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  NPT_CHECK_LABEL(m_delegate->m_transport->GetStateVariableValue("TransportState", data), failed);
  return data != "STOPPED";
failed:
  return false;
}

bool CUPnPPlayer::IsPaused() const
{
  NPT_String data;
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  NPT_CHECK_LABEL(m_delegate->m_transport->GetStateVariableValue("TransportState", data), failed);
  return data == "PAUSED_PLAYBACK";
failed:
  return false;
}

void CUPnPPlayer::SetVolume(float volume)
{
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  NPT_CHECK_LABEL(m_control->SetVolume(m_delegate->m_device
                                     , m_delegate->m_instance
                                     , "Master", (int)(volume * 100)
                                     , m_delegate), failed);
  return;
failed:
  CLog::Log(LOGERROR, "UPNP: CUPnPPlayer - unable to set volume");
  return;
}

int64_t CUPnPPlayer::GetTime()
{
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  return m_delegate->m_posinfo.rel_time.ToMillis();
failed:
  return 0;
}

int64_t CUPnPPlayer::GetTotalTime()
{
  NPT_CHECK_POINTER_LABEL_SEVERE(m_delegate, failed);
  return m_delegate->m_posinfo.track_duration.ToMillis();
failed:
  return 0;
};

CStdString CUPnPPlayer::GetPlayingTitle()
{
  return "";
};

} /* namespace UPNP */

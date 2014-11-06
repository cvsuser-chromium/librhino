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

#include "AnnouncementManager.h"
#include "base/lazy_instance.h"
#include "threads/SingleLock.h"
#include <stdio.h>
#include "utils/log.h"
#include "utils/Variant.h"

#define LOOKUP_PROPERTY "database-lookup"

using namespace std;
using namespace ANNOUNCEMENT;

namespace {
base::LazyInstance<ANNOUNCEMENT::CAnnouncementManager::Globals> g_announcementManager_globals;
}
#define m_announcers g_announcementManager_globals.Get().m_announcers
//XBMC_GLOBAL_USE(ANNOUNCEMENT::CAnnouncementManager::Globals).m_announcers
#define m_critSection g_announcementManager_globals.Get().m_critSection
//XBMC_GLOBAL_USE(ANNOUNCEMENT::CAnnouncementManager::Globals).m_critSection

void CAnnouncementManager::AddAnnouncer(IAnnouncer *listener)
{
  if (!listener)
    return;

  CSingleLock lock (m_critSection);
  m_announcers.push_back(listener);
}

void CAnnouncementManager::RemoveAnnouncer(IAnnouncer *listener)
{
  if (!listener)
    return;

  CSingleLock lock (m_critSection);
  for (unsigned int i = 0; i < m_announcers.size(); i++)
  {
    if (m_announcers[i] == listener)
    {
      m_announcers.erase(m_announcers.begin() + i);
      return;
    }
  }
}

void CAnnouncementManager::Announce(AnnouncementFlag flag, const char *sender, const char *message)
{
  Announce(flag, sender, message, "");
}

void CAnnouncementManager::Announce(AnnouncementFlag flag, const char *sender, const char *message, const std::string& json_data)
{
  LOG(INFO) << "CAnnouncementManager - Announcement: " << message << " from " << sender;
  CSingleLock lock (m_critSection);
  for (unsigned int i = 0; i < m_announcers.size(); i++)
    m_announcers[i]->Announce(flag, sender, message, json_data);
}
#if 0
void CAnnouncementManager::Announce(AnnouncementFlag flag, const char *sender, const char *message, CFileItemPtr item)
{
  CVariant data;
  Announce(flag, sender, message, item, data);
}

void CAnnouncementManager::Announce(AnnouncementFlag flag, const char *sender, const char *message, CFileItemPtr item, CVariant &data)
{
  if (!item.get())
  {
    Announce(flag, sender, message, data);
    return;
  }

  // Extract db id of item
  CVariant object = data.isNull() || data.isObject() ? data : CVariant::VariantTypeObject;
  CStdString type;
  int id = 0;

  if(item->HasPVRChannelInfoTag())
  {
    const PVR::CPVRChannel *channel = item->GetPVRChannelInfoTag();
    id = channel->ChannelID();
    type = "channel";

    object["item"]["title"] = channel->ChannelName();
    object["item"]["channeltype"] = channel->IsRadio() ? "radio" : "tv";

    if (data.isMember("player") && data["player"].isMember("playerid"))
      object["player"]["playerid"] = channel->IsRadio() ? PLAYLIST_MUSIC : PLAYLIST_VIDEO;
  }
  else if (item->HasVideoInfoTag())
  {
    id = item->GetVideoInfoTag()->m_iDbId;

    // TODO: Can be removed once this is properly handled when starting playback of a file
    if (id <= 0 && !item->GetPath().empty() &&
       (!item->HasProperty(LOOKUP_PROPERTY) || item->GetProperty(LOOKUP_PROPERTY).asBoolean()))
    {
      CVideoDatabase videodatabase;
      if (videodatabase.Open())
      {
        CStdString path = item->GetPath();
        CStdString videoInfoTagPath(item->GetVideoInfoTag()->m_strFileNameAndPath);
        if (videoInfoTagPath.Find("removable://") == 0)
          path = videoInfoTagPath;
        if (videodatabase.LoadVideoInfo(path, *item->GetVideoInfoTag()))
          id = item->GetVideoInfoTag()->m_iDbId;

        videodatabase.Close();
      }
    }

    if (!item->GetVideoInfoTag()->m_type.empty())
      type = item->GetVideoInfoTag()->m_type;
    else
      CVideoDatabase::VideoContentTypeToString((VIDEODB_CONTENT_TYPE)item->GetVideoContentType(), type);

    if (id <= 0)
    {
      // TODO: Can be removed once this is properly handled when starting playback of a file
      item->SetProperty(LOOKUP_PROPERTY, false);

      CStdString title = item->GetVideoInfoTag()->m_strTitle;
      if (title.IsEmpty())
        title = item->GetLabel();
      object["item"]["title"] = title;

      switch (item->GetVideoContentType())
      {
      case VIDEODB_CONTENT_MOVIES:
        if (item->GetVideoInfoTag()->m_iYear > 0)
          object["item"]["year"] = item->GetVideoInfoTag()->m_iYear;
        break;
      case VIDEODB_CONTENT_EPISODES:
        if (item->GetVideoInfoTag()->m_iEpisode >= 0)
          object["item"]["episode"] = item->GetVideoInfoTag()->m_iEpisode;
        if (item->GetVideoInfoTag()->m_iSeason >= 0)
          object["item"]["season"] = item->GetVideoInfoTag()->m_iSeason;
        if (!item->GetVideoInfoTag()->m_strShowTitle.empty())
          object["item"]["showtitle"] = item->GetVideoInfoTag()->m_strShowTitle;
        break;
      case VIDEODB_CONTENT_MUSICVIDEOS:
        if (!item->GetVideoInfoTag()->m_strAlbum.empty())
          object["item"]["album"] = item->GetVideoInfoTag()->m_strAlbum;
        if (!item->GetVideoInfoTag()->m_artist.empty())
          object["item"]["artist"] = StringUtils::Join(item->GetVideoInfoTag()->m_artist, " / ");
        break;
      }
    }
  }
  else if (item->HasMusicInfoTag())
  {
    id = item->GetMusicInfoTag()->GetDatabaseId();
    type = "song";

    // TODO: Can be removed once this is properly handled when starting playback of a file
    if (id <= 0 && !item->GetPath().empty() &&
       (!item->HasProperty(LOOKUP_PROPERTY) || item->GetProperty(LOOKUP_PROPERTY).asBoolean()))
    {
      CMusicDatabase musicdatabase;
      if (musicdatabase.Open())
      {
        CSong song;
        if (musicdatabase.GetSongByFileName(item->GetPath(), song, item->m_lStartOffset))
        {
          item->GetMusicInfoTag()->SetSong(song);
          id = item->GetMusicInfoTag()->GetDatabaseId();
        }

        musicdatabase.Close();
      }
    }

    if (id <= 0)
    {
      // TODO: Can be removed once this is properly handled when starting playback of a file
      item->SetProperty(LOOKUP_PROPERTY, false);

      CStdString title = item->GetMusicInfoTag()->GetTitle();
      if (title.IsEmpty())
        title = item->GetLabel();
      object["item"]["title"] = title;

      if (item->GetMusicInfoTag()->GetTrackNumber() > 0)
        object["item"]["track"] = item->GetMusicInfoTag()->GetTrackNumber();
      if (!item->GetMusicInfoTag()->GetAlbum().empty())
        object["item"]["album"] = item->GetMusicInfoTag()->GetAlbum();
      if (!item->GetMusicInfoTag()->GetArtist().empty())
        object["item"]["artist"] = item->GetMusicInfoTag()->GetArtist();
    }
  }
  else if (item->IsVideo())
  {
    // video item but has no video info tag.
    type = "movies";
    object["item"]["title"] = item->GetLabel();
  }
  else if (item->HasPictureInfoTag())
  {
    type = "picture";
    object["item"]["file"] = item->GetPath();
  }
  else
    type = "unknown";

  object["item"]["type"] = type;
  if (id > 0)
    object["item"]["id"] = id;

  Announce(flag, sender, message, object);
}
#endif

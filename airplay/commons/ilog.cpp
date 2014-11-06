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
#include "base/strings/stringprintf.h"
#include "commons/ilog.h"
//#include "utils/StdString.h"

namespace XbmcCommons
{
  void ILogger::Log(int loglevel, const char *format, ... )
  {
    std::string strData;

    strData.reserve(16384);
    va_list va;
    va_start(va, format);
    //strData.FormatV(format,va);
    base::StringAppendV(&strData, format, va);
    va_end(va);

    //log(loglevel, strData);
    LOG(INFO) << strData;
  }
}

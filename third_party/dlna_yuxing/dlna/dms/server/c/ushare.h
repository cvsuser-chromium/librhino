/*
 * ushare.h : GeeXboX uShare UPnP Media Server header.
 * Originally developped for the GeeXboX project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _USHARE_H_
#define _USHARE_H_

#include "upnp.h"
#include "upnptools.h"
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "LinkedList.h"

#ifdef HAVE_DLNA
#include <dlna.h>
#endif /* HAVE_DLNA */

#include "buffer.h"
#include "redblack.h"
#include "dms.h"

#define VIRTUAL_DIR "/web"
#define UPNP_MAX_AGE 3600 //senconds
#define STARTING_ENTRY_ID_DEFAULT 0

#define UPNP_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <device>" \
"    <deviceType>urn:schemas-upnp-org:device:MediaServer:1</deviceType>" \
"    <friendlyName>%s</friendlyName>" \
"    <manufacturer>yuxing software</manufacturer>" \
"    <manufacturerURL>http://www.yuxingsoft.com/</manufacturerURL>" \
"    <modelDescription>DMS for Huawei</modelDescription>" \
"    <modelName>%s</modelName>" \
"    <modelNumber>001</modelNumber>" \
"    <modelURL>http://www.yuxing.com/</modelURL>" \
"    <serialNumber>YX5555</serialNumber>" \
"    <UDN>uuid:%s</UDN>" \
"    <serviceList>" \
"      <service>" \
"        <serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>" \
"        <serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>" \
"        <SCPDURL>/web/cms.xml</SCPDURL>" \
"        <controlURL>/web/cms_control</controlURL>" \
"        <eventSubURL>/web/cms_event</eventSubURL>" \
"      </service>" \
"      <service>" \
"        <serviceType>urn:schemas-upnp-org:service:ContentDirectory:1</serviceType>" \
"        <serviceId>urn:upnp-org:serviceId:ContentDirectory</serviceId>" \
"        <SCPDURL>/web/cds.xml</SCPDURL>" \
"        <controlURL>/web/cds_control</controlURL>" \
"        <eventSubURL>/web/cds_event</eventSubURL>" \
"      </service>" \
"      <service>" \
"        <serviceType>urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1</serviceType>\n" \
"        <serviceId>urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar</serviceId>\n" \
"        <SCPDURL>/web/msr.xml</SCPDURL>" \
"        <controlURL>/web/msr_control</controlURL>" \
"        <eventSubURL>/web/msr_event</eventSubURL>" \
"      </service>\n" \
"    </serviceList>" \
"    <presentationURL>/web/ushare.html</presentationURL>" \
"  </device>" \
"</root>"


struct ushare_t {
	char 				*friendlyName;
	char				*manufacturer;
	char				*manufacturerURL;

	char				*modelDescription;
	char				*modelName;
	char				*modelNumber;
	char				*modelURL;
	char				*serialNumber;
	char				*icon;
	
	char 				*interface;
	char 				*udn;
	char 				*ip;
	unsigned short 		port;
	UpnpDevice_Handle	dev;

	C_DMS_VO 			*root_entry;
	unsigned int		systemUpdateId;
	struct rbtree 		*rb;
	int					object_total_limit;
	int					total_entries;
	int 				nr_entries;
	int 				starting_id;
	int 				init;
	
	bool				dlna_enabled;
	dlna_t 				*dlna;
	dlna_org_flags_t	dlna_flags;

	int					stop_http;

//	struct buffer_t		*presentation;
	bool				verbose;
};

struct action_event_t {
	struct Upnp_Action_Request	*request;
	bool 						status;
	struct service_t			*service;
};

#define DMS_OBJCT_IS_PVR_VOD(x)		(x->source_type == e_SOURCE_PVR || x->source_type == e_SOURCE_VOD)

void Dms_WaitLock(void);
void Dms_PostLock(void);

#endif /* _USHARE_H_ */


/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* $Id: device.h 261 2006-08-26 14:17:41Z r3mi $
 *
 * UPnP Device
 * This file is part of djmount.
 *
 * (C) Copyright 2005 R�mi Turboult <r3mi@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef DEVICE_INCLUDED
#define DEVICE_INCLUDED 1

#include <stdbool.h>

#include "upnp.h"
#include "ixml.h"
#include "LinkedList.h"
//#include "service.h"

#ifdef __cplusplus
extern "C" {
#endif

// ContentDirectory ServiceType
#define CONTENT_DIR_SERVICE_TYPE \
	"urn:schemas-upnp-org:service:ContentDirectory:1"

#define CDS_TYPE_BASE \
	"urn:schemas-upnp-org:service:ContentDirectory:"
#define CDS_TYPE_VERSION 1

#define CDS_TYPE_MATCH(service)	\
	( service && !strncasecmp(service, CDS_TYPE_BASE, strlen(CDS_TYPE_BASE)) && \
	 ( atoi(&(service[strlen(CDS_TYPE_BASE)])) >= CDS_TYPE_VERSION) )

// DMS type
#define DMS_TYPE_BASE \
	"urn:schemas-upnp-org:device:MediaServer:"
#define DMS_TYPE_VERSION 1

#define DMS_TYPE_MATCH(device)	\
	( device && !strncasecmp(device, DMS_TYPE_BASE, strlen(DMS_TYPE_BASE) ) && \
	  ( atoi(&(device[strlen(DMS_TYPE_BASE)])) >= DMS_TYPE_VERSION ) )

/******************************************************************************
 * @var Device
 *
 *	This opaque type encapsulates access to the UPnP device.
 *	
 *      NOTE THAT THE FUNCTION API IS NOT THREAD SAFE. Function should 
 *      be called through the global functions in "upnp_devicelist.h", 
 *	who lock the global device list.
 *
 *****************************************************************************/

struct _Device;
typedef struct _Device Device;

struct _Device {

	time_t		creation_time;

	/*
	 * <root> elements
	 */
	IXML_Document*	descDoc;
	const char*	descDocURL;
	const char*	descDocText;
	const char*	baseURL;	// <URLBase> if exists, else descDocURL

	/*
	 * <device> elements
	 */
	const IXML_Node*  descDocNode;	// pointer to this <device> in descDoc
	const char*	udn; 		// <UDN> == deviceId
	const char*	deviceType; 	// <deviceType>
	const char*	friendlyName; 	// <friendlyName>
	const char*	presURL;	// <presentationURL> resolved with base
	
	LinkedList	services; 	// Linked list of Service*
	int			ip;
};

/******************************************************************************
 * @brief Creates a new device.
 *	This routine parse the DOM description document for the device
 *	(which can be the root device, or any embedded device in the document).
 *	The returned Device* can be destroyed with "talloc_free".
 *
 * @param context        the talloc parent context
 * @param ctrlpt_handle  the UPnP client handle
 * @param descDocURL 	 the URL of the description document 
 * @param deviceId	 device UDN, or NULL to default to root device
 * @param descDocText 	 the XML text of the description document
 *
 *****************************************************************************/
Device* 
Device_Create (void* context, 
	       UpnpClient_Handle ctrlpt_handle, 
	       const char* const descDocURL, 
	       const char* const deviceId,
	       const char* const descDocText);


/******************************************************************************
 * @brief Subscribe all services to their event URL
 *****************************************************************************/
int
Device_SusbcribeAllEvents (const Device* dev);


/** 
 * @brief Returns the value of an element from the Device Description.
 *	The searched element shall be a direct child of the <device> element
 *	(no recursive seach is done), and only the 1st element found is 
 *	returned for a multi-valued element.
 * 	The returned string is internal to the Device, and should be copied 
 *	if necessary e.g. if the Device is to be destroyed.	
 */
const char*
Device_GetDescDocItem (const Device* dev, const char* tagname, bool log_error);


/** 
 * @brief Returns a copy of the text of the XML Device Description Document.
 * 	  The returned string should be freed using "talloc_free".
 */
char*
Device_GetDescDocTextCopy (const Device* dev, void* result_context);


/******************************************************************************
 * @fn	    Device_GetServiceFrom
 * @brief   returns the pointer to a given Service.
 * 	Given a service name, returns the pointer to the matching 
 *	service. The service can be identified from various strings.
 *	Note that this function is not thread safe.  It must be called 
 *      from a function that has locked the global device list.
 *
 * @param dev	    the parent device
 * @param servname  the service name
 * @param from      the kind of service name
 * @param log_error if true, log an error if service can't be found
 *
 *****************************************************************************/
enum GetFrom { FROM_SID, FROM_CONTROL_URL, FROM_EVENT_URL, FROM_SERVICE_TYPE };
#if 0
Service* 
Device_GetServiceFrom (const Device* dev, 
		       const char* servname, enum GetFrom from,
		       bool log_error);

#endif
/******************************************************************************
 * @brief Returns a string describing the state of the device.
 * 	  The returned string should be freed using "talloc_free".
 *	  If 'debug' is true, returns extra debugging information (which
 *	  might need to be computed).
 *****************************************************************************/
char*
Device_GetStatusString (const Device* dev, void* result_context,
			bool debug);


void Device_parse_stamp_xml(void);
int Device_get_dms_stamp(const Device* dev);

#ifdef __cplusplus
}; // extern "C"
#endif


#endif /* DEVICE_INCLUDED */





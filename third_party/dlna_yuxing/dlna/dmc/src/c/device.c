/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* $Id: device.c 261 2006-08-26 14:17:41Z r3mi $
 *
 * UPnP Device
 * This file is part of djmount.
 *
 * (C) Copyright 2005 R�mi Turboult <r3mi@users.sourceforge.net>
 *
 * Part derived from libupnp example (upnp/sample/tvctrlpt/upnp_tv_ctrlpt.c)
 * Copyright (c) 2000-2003 Intel Corporation
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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "device.h"
#include "service.h"
#include "upnp_util_dmc.h"
#include "xml_util.h"
#include "log.h"
#include "talloc_util.h"

#include <time.h>
#include <stdbool.h>
//#include "upnp.h"
//#include "LinkedList.h"

#include "hitTime.h"

/*****************************************************************************
 * @fn	ServiceFactory
 *	Creates a Service object, whose class depends on the service type.
 *****************************************************************************/

#include "service.h"
//#include "content_dir.h"

static Service* 
ServiceFactory (Device* dev,
		UpnpClient_Handle ctrlpt_handle, 
		IXML_Element* serviceDesc, 
		const char* base_url)
{
	Service* serv = NULL;
	/*
	 * Simple implementation, hardcoding the 2 possible classes of Service.
	 */
	const char* const serviceType = XMLUtil_FindFirstElementValue
		(XML_E2N (serviceDesc), "serviceType", false, true);

	if ( serviceType && CDS_TYPE_MATCH(serviceType) ) {
#if 0		
		serv = ContentDir_ToService 
			(ContentDir_Create (dev, ctrlpt_handle, 
					    serviceDesc, base_url));
	} else {
#endif	
		serv = Service_Create (dev, ctrlpt_handle,
				       serviceDesc, base_url);
	}
	if (serv == NULL)
		Log_Printf (LOG_ERROR, "Error creating service type %s",
			    NN(serviceType));
	return serv;
}


/******************************************************************************
 * Find the correct <device> XML Node, defaulting to root device if not found,
 * or if deviceId not specified.
 *****************************************************************************/
static const IXML_Node*
findDeviceNode (const char* const deviceId, IXML_Document* const descDoc)
{
	// Get all <device>'s : the root device, and any embedded device
	// in <deviceList>'s
	IXML_NodeList* devicesList = 
		ixmlDocument_getElementsByTagName (descDoc, "device");
	if (ixmlNodeList_length (devicesList) < 1) {
		ixmlNodeList_free (devicesList);
		return NULL; // ---------->
	}
	
	// Find the correct <device>, defaulting to root device if not found,
	// or if deviceId not specified.
	const IXML_Node* descDocNode = NULL;
	if (deviceId) {
		int i;
		for (i = 0; i < ixmlNodeList_length (devicesList); i++) {
			IXML_Node* node = ixmlNodeList_item (devicesList, i);
			const char* udn = XMLUtil_FindFirstElementValue
				(node, "UDN", false, true);
			if (udn && strcmp (udn, deviceId) == 0) {
				descDocNode = node;
				break; // ---------->
			}
		}
		if (descDocNode == NULL) {
			Log_Printf (LOG_WARNING,
				    "Device_Create can't find UDN='%s' in XML "
				    "document, defaulting to root device",
				    deviceId);
		}
	}
	if (descDocNode == NULL) 
		descDocNode = ixmlNodeList_item (devicesList, 0);
	ixmlNodeList_free (devicesList);
	
	return descDocNode;
}


/******************************************************************************
 * destroy
 *
 * Description: 
 *	Device destructor, automatically called by "talloc_free".
 *
 *****************************************************************************/
static int
destroy (Device* const dev)
{
	if (dev) {
		/* Delete list.
		 * Note that items are not destroyed : Service* are 
		 * automatically deallocated by "talloc" when parent Device 
		 * is detroyed.
		 */
		ListDestroy (&dev->services, /*freeItem=>*/ 0); 

		// Delete description document
		if (dev->descDoc) {
			ixmlDocument_free (dev->descDoc);
			dev->descDoc = NULL;
		}  

		// Reset all pointers to NULL 
		*dev = (struct _Device) { };
    
		// The "talloc'ed" strings will be deleted automatically 
	}
	return 0; // ok -> deallocate memory
}


/*****************************************************************************
 * Device_Create
 *****************************************************************************/

Device* 
Device_Create (void* parent_context, 
	       UpnpClient_Handle ctrlpt_handle, 
	       const char* const descDocURL, 
	       const char* const deviceId,
	       const char* const descDocText)
{
	if (descDocURL == NULL || *descDocURL == NUL) {
		Log_Printf (LOG_ERROR, 
			    "NULL or empty description document URL");
		return NULL; // ---------->
	}
	if (descDocText == NULL) {
		Log_Printf (LOG_ERROR, 
			    "NULL description document XML text");
		return NULL; // ---------->
	}

	Log_Printf (LOG_DEBUG, "Device_Create : Id = '%s', "
		    "description document = "
		    "--------------------\n%s\n--------------------",
		    NN(deviceId), descDocText);

	IXML_Document* descDoc = NULL;
	int rc = ixmlParseBufferEx (discard_const_p (char, descDocText), 
				    &descDoc);
	if (rc != IXML_SUCCESS) {
		Log_Printf (LOG_ERROR, "Device_Create can't parse XML "
			    "document (%d) = '%s'", rc, descDocText);
		return NULL; // ---------->
	}
	
	/*
	 * Find the correct <device>, defaulting to root device if not found,
	 * or if deviceId not specified.
	 */
	const IXML_Node* const descDocNode = findDeviceNode(deviceId, descDoc);
	if (descDocNode == NULL) {
		Log_Printf (LOG_ERROR, 
			    "Device_Create no <device> in XML document = '%s'",
			    descDocText);
		ixmlDocument_free (descDoc);
		return NULL; // ---------->
	}

	/*
	 * Create the Device object
	 */
	Device* const dev = talloc (parent_context, Device);
	if (dev == NULL) {
		Log_Print (LOG_ERROR, "Device_Create Out of Memory");
		ixmlDocument_free (descDoc);
		return NULL; // ---------->
	}
	
	*dev = (struct _Device) { 
		.creation_time = time (NULL),
		.descDocURL    = talloc_strdup (dev, descDocURL),
		.descDoc       = descDoc,
		.descDocText   = talloc_strdup (dev, descDocText),
		.descDocNode   = descDocNode,
		// Other fields to empty values
	};

	const char* const baseURL = XMLUtil_FindFirstElementValue
		(XML_D2N(dev->descDoc), "URLBase", true, false);
	dev->baseURL = ( (baseURL && baseURL[0]) ? baseURL : dev->descDocURL );
	
	
	/*
	 * Read key elements from <device> description document 
	 */
	dev->udn = talloc_strdup (dev, Device_GetDescDocItem (dev, "UDN", 
							      true));
	Log_Printf (LOG_DEBUG, "Device_Create : UDN = %s", dev->udn);

	dev->deviceType = talloc_strdup (dev, Device_GetDescDocItem 
					 (dev, "deviceType", true));
	Log_Printf (LOG_DEBUG, "Device_Create : type = %s", dev->deviceType);
	
	dev->friendlyName = talloc_strdup (dev, Device_GetDescDocItem 
					   (dev, "friendlyName", true));

	const char* const relURL  = Device_GetDescDocItem (dev, 
							   "presentationURL",
							   false);
	dev->presURL = UpnpUtil_ResolveURL (dev, dev->baseURL, relURL);
	
	/*
	 * Find and parse services
	 */
	ListInit (&dev->services, NULL, NULL);
	
	IXML_Element* const serviceList = XMLUtil_FindFirstElement
		(dev->descDocNode, "serviceList", false, false);
	IXML_NodeList* services = ixmlElement_getElementsByTagName
		(serviceList, "service");
	int i;
	for (i = 0; i < ixmlNodeList_length (services); i++ ) {
		IXML_Element* const serviceDesc = 
			(IXML_Element *) ixmlNodeList_item (services, i);
		Service* const serv = ServiceFactory (dev, ctrlpt_handle, 
						      serviceDesc, 
						      dev->baseURL);
		if (serv)
			ListAddTail (&dev->services, serv);
	}
	
	if (serviceList) {
		ixmlNodeList_free (services);
		services = NULL;
	}  

	// Register destructor
	talloc_set_destructor (dev, destroy);
	
	return dev;
}


/*****************************************************************************
 * Device_GetDescDocItem
 *****************************************************************************/
const char*
Device_GetDescDocItem (const Device* dev, const char* tagname, bool log_error)
{
	if (dev)
		return XMLUtil_FindFirstElementValue 
			(dev->descDocNode, tagname, false, log_error);
	else 
		return NULL;
}


/*****************************************************************************
 * Device_GetDescDocTextCopy
 *****************************************************************************/
char*
Device_GetDescDocTextCopy (const Device* dev, void* result_context)
{
	return (dev ? talloc_strdup (result_context, dev->descDocText) : NULL);
}


/*****************************************************************************
 * Device_SusbcribeAllEvents
 *****************************************************************************/

int
Device_SusbcribeAllEvents (const Device* dev)
{  
	if (dev == NULL)
		return UPNP_E_INVALID_DEVICE; // ---------->

	Log_Printf (LOG_DEBUG, "Device_SusbcribeAllEvents %s",
		    NN(dev->friendlyName));

	int rc = UPNP_E_SUCCESS;
	ListNode* node;
	LinkedList* const services = discard_const_p (LinkedList, 
						      &dev->services);
	for (node = ListHead (services);
	     node != NULL;
	     node = ListNext (services, node)) {
		int rc2 = Service_SubscribeEventURL (node->item); 
		if (rc2 != UPNP_E_SUCCESS)
			rc = rc2;
	}
	return rc;
}


/*****************************************************************************
 * Device_GetService
 *****************************************************************************/

Service*
Device_GetServiceFrom (const Device* dev, const char* servname, enum GetFrom from, bool log_error)
{  
	if (servname) {
		ListNode* node;
		LinkedList* const services = discard_const_p (LinkedList, 
							      &dev->services);
		for (node = ListHead (services);
		     node != NULL;
		     node = ListNext (services, node)) {
			const char* s = NULL;
			switch (from) {
			case FROM_SID:		
				s = Service_GetSid (node->item); break;
			case FROM_CONTROL_URL:	
				s = Service_GetControlURL (node->item); break;
			case FROM_EVENT_URL:	
				s = Service_GetEventURL (node->item); break;
			case FROM_SERVICE_TYPE:	
				s = Service_GetServiceType (node->item); 
				if (s && CDS_TYPE_MATCH(s) )
					return node->item;
				break;
			}
			if (s && strcmp (servname, s) == 0) 
				return node->item; // ---------->*/
		}
	}
	if (log_error)
		Log_Printf (LOG_ERROR, 
			    "Device '%s' : error finding Service '%s'",
			    NN(dev->friendlyName), NN(servname));
	return NULL;
}

#if 0
#endif


/* $Id: upnp_util.h 250 2006-08-13 17:54:22Z r3mi $
 *
 * UPnP Utilities
 * This file is part of djmount.
 *
 * (C) Copyright 2005 R�mi Turboult <r3mi@users.sourceforge.net>
 *
 * Part derived from libupnp example (upnp/sample/common/sample_util.h)
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

#ifndef UPNP_UTIL_H_INCLUDED
#define UPNP_UTIL_H_INCLUDED


#include "upnp.h"
#include "upnptools.h"


#ifdef __cplusplus
extern "C" {
#endif



/*****************************************************************************
 * Returns a string with callback event structure details.
 * The returned string should be freed using "talloc_free".
 *
 * @param talloc_context   talloc context to allocate new string (may be 0)
 * @param eventType        the type of callback event	
 * @param event            the callback event structure
 *****************************************************************************/
char*
UpnpUtil_GetEventString (void* talloc_context,
			 IN Upnp_EventType eventType, 
			 IN const void* event);


/*****************************************************************************
 * Returns a string with the name of a Upnp_EventType.
 * @return 	a static constant string, or NULL if unknown event type.
 *****************************************************************************/
const char*
UpnpUtil_GetEventTypeString (IN Upnp_EventType e);



/******************************************************************************
 * Combines a base URL and a relative URL into a single absolute URL.
 * Similar to "UpnpResolveURL" but allocates the memory for the result
 * (the returned string should be freed using "talloc_free").
 *
 * @param talloc_context  parent context to allocate result, may be NULL
 * @param baseURL	  the base URL to combine
 * @param relURL	  the URL relative to baseURL
 * @param return   	  the absolute URL if ok, or "" if error.
 *****************************************************************************/
char*
UpnpUtil_ResolveURL (void* talloc_context,
		     const char* baseURL, const char* relURL);



#ifdef __cplusplus
}; // extern "C"
#endif



#endif // UPNP_UTIL_H_INCLUDED




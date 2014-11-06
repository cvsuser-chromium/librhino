/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* $Id: device_list.c 265 2006-08-27 17:53:14Z r3mi $
 *
 * DeviceList : List of UPnP Devices
 * This file is part of djmount.
 *
 * (C) Copyright 2005 Rémi Turboult <r3mi@users.sourceforge.net>
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

#include "device_list.h"
//#include "device.h"
#include "upnp_util_dmc.h"
#include "log.h"
#include "service.h"
#include "talloc_util.h"

#include <stdbool.h>
#include "upnp.h"
#include "ithread.h"
#include "upnptools.h"
#include "LinkedList.h"
//#include "content_dir.h"

#include "hitTime.h"

#ifdef	ENABLE_HT_PRINTF
#define HT_PAR_DEFAULT		HT_MOD_DMC, HT_BIT_FEW, __FILE__, __LINE__, __FUNCTION__
#define EF_DBG_FUNC_START(x, str)	HT_Printf(HT_PAR_DEFAULT, "%s start: %s %d.\n\r", __FUNCTION__, ((str)?(str):"null"), (int)(x) )
#define EF_DBG_FUNC_END(x, str)		HT_Printf(HT_PAR_DEFAULT, "%s   end: %s %d.\n\r", __FUNCTION__, ((str)?(str):"null"), (int)(x) )
#define EF_DBG_FUNC(x, str)			HT_Printf(HT_PAR_DEFAULT, "%s      : %s %d.\n\r", __FUNCTION__, ((str)?(str):"null"), (int)(x) )
#else
#define EF_DBG_FUNC_START(x, str)	{}
#define EF_DBG_FUNC_END(x, str)	{}
#define EF_DBG_FUNC(x, str)	{}

#endif

// How often to check advertisement and subscription timeouts for devices
static const unsigned int CHECK_SUBSCRIPTIONS_TIMEOUT = 30; // in seconds



static UpnpClient_Handle g_ctrlpt_handle = -1;


static ithread_t g_timer_thread;

static char* g_ssdp_target = NULL;


/*
 * Mutex for protecting the global device list
 * in a multi-threaded, asynchronous environment.
 * All functions should lock this mutex before reading
 * or writing the device list.
 */
static ithread_mutex_t DeviceListMutex;

#if 0

#endif

//
// TBD XXX TBD
// TBD to replace with Hash Table for better performances XXX
// TBD XXX TBD
//
//static LinkedList GlobalDeviceList;
LinkedList GlobalDeviceList;

void DeviceList_Lock(void)
{
  //	EF_DBG_FUNC(1, NULL);
  ithread_mutex_lock (&DeviceListMutex);
}
void DeviceList_Unlock (void)
{
  //	EF_DBG_FUNC(-1, NULL);
  ithread_mutex_unlock (&DeviceListMutex);
}


/*****************************************************************************
 * NotifyUpdate
 *
 * Description: callback for update events
 *
 * Parameters:
 *
 *****************************************************************************/

// Function pointer for update event callbacks
static DeviceList_EventCallback gStateUpdateFun = 0;
static ServiceUpdatedCallback gServiceUpdateCallback = 0;

static void
NotifyUpdate (DeviceList_EventType type,
  // TBD	      const char* varName,
  // TBD	      const char* varValue,
  const DeviceNode* devnode)
{
  if(devnode && devnode->d )
  {
    EF_DBG_FUNC_START(type, talloc_get_name (devnode->d));
  }
  else
  {
    EF_DBG_FUNC_START(type, "!!!!!!!!!!!!!!!!!! no node");
  }

  if (gStateUpdateFun && devnode && devnode->d)
    //gStateUpdateFun (type, devnode->deviceId, talloc_get_name (devnode->d));
    gStateUpdateFun (type, devnode->d, devnode->deviceId, talloc_get_name (devnode->d));
  // TBD: Add mutex here?
}

void Service_UpdatedCallback(char *dmsUdn, char *name, char *value)
{
  if(gServiceUpdateCallback)
    gServiceUpdateCallback(dmsUdn, name, value);
}
/*****************************************************************************
 * GetDeviceNodeFromName
 *
 * Description:
 *       Given a device name, returns the pointer to the device node.
 *       Note that this function is not thread safe.  It must be called
 *       from a function that has locked the global device list.
 *
 * @param name 	the device name
 *
 *****************************************************************************/

static DeviceNode*
GetDeviceNodeFromName (const char* name, bool log_error)
{
  if (name) {
    ListNode* node;
    for (node = ListHead (&GlobalDeviceList);
      node != 0;
      node = ListNext (&GlobalDeviceList, node)) {
      DeviceNode* const devnode = node->item;
      if (devnode && devnode->d &&
        strcmp (talloc_get_name (devnode->d), name) == 0)
        return devnode; // ---------->
    }
  }
  if (log_error)
    Log_Printf (LOG_ERROR, "Error finding Device named %s", NN(name));
  return 0;
}

static ListNode*
GetDeviceListNodeFromId (const char* deviceId)
{
  if (deviceId) {
    ListNode* node;
    for (node = ListHead (&GlobalDeviceList);
      node != 0;
      node = ListNext (&GlobalDeviceList, node)) {
      DeviceNode* const devnode = node->item;
      if (devnode && devnode->deviceId &&
        strcmp (devnode->deviceId, deviceId) == 0)
        return node; // ---------->
    }
  }
  return 0;
}
Device*
GetDeviceByUdn (const char* udn)
{
  if (udn) {
    ListNode* node;
    for (node = ListHead (&GlobalDeviceList);
      node != 0;
      node = ListNext (&GlobalDeviceList, node)) {
      DeviceNode* const devnode = node->item;
      if (devnode && devnode->d && (strcmp (devnode->d->udn, udn) == 0) )
        return devnode->d; // ---------->
    }
  }
  return 0;
}

static Service*
GetService (const char* s, enum GetFrom from)
{
  ListNode* node;
  for (node = ListHead (&GlobalDeviceList);
    node != NULL;
    node = ListNext (&GlobalDeviceList, node)) {
    DeviceNode* const devnode = node->item;
    if (devnode) {
      Service* const serv = Device_GetServiceFrom(devnode->d, s, from, false);
      if (serv)
        return serv; // ---------->
    }
  }
  Log_Printf (LOG_ERROR, "Can't find service matching %s in device list",
    NN(s));
  return NULL;
}

static char *GetUdnBySid (const char* sid)
{
  ListNode* node;
  for (node = ListHead (&GlobalDeviceList);
    node != NULL;
    node = ListNext (&GlobalDeviceList, node)) {
    DeviceNode* const devnode = node->item;
    if (devnode) {
      Service* const serv = Device_GetServiceFrom(devnode->d, sid, FROM_SID, false);
      if (serv)
        return devnode->deviceId; // ---------->
    }
  }
  return NULL;
}


/*****************************************************************************
 * make_device_name
 *
 * Generate a unique device name.
 * Must be deallocated with "talloc_free".
 *****************************************************************************/
static char*
make_device_name (void* talloc_context, const char* base)
{
  if (base == 0 || *base == '\0')
    base = "unnamed";
  char* name = String_CleanFileName (talloc_context, base);

  char* res = name;
  int i = 1;
  while (GetDeviceNodeFromName (res, false)) {
    if (res != name)
      talloc_free (res);
    res = talloc_asprintf (talloc_context, "%s_%d", name, ++i);
  }
  if (res != name)
    talloc_free (name);

  return res;
}


/*****************************************************************************
 * DeviceList_RemoveDevice
 *
 * Description:
 *       Remove a device from the global device list.
 *
 * Parameters:
 *   deviceId -- The Unique Device Name for the device to remove
 *
 *****************************************************************************/
int
DeviceList_RemoveDevice (const char* deviceId)
{
  int rc = UPNP_E_SUCCESS;

  //	EF_DBG_FUNC_START(0, deviceId);
  DeviceList_Lock();

  ListNode* const node = GetDeviceListNodeFromId (deviceId);
  if (node) {
    DeviceNode* devnode = node->item;
    node->item = 0;
    ListDelNode (&GlobalDeviceList, node, /*freeItem=>*/ 0);
    // Do the notification while the global list is still locked
    NotifyUpdate (E_DEVICE_REMOVED, devnode);
    talloc_free (devnode);
  } else {
    Log_Printf (LOG_WARNING, "RemoveDevice can't find Id=%s",
      NN(deviceId));
    rc = UPNP_E_INVALID_DEVICE;
  }

  DeviceList_Unlock();

  //	EF_DBG_FUNC_END(0, 0);

  return rc;
}


/*****************************************************************************
 * DeviceList_RemoveAll
 *
 * Description:
 *       Remove all devices from the global device list.
 *
 * Parameters:
 *   None
 *
 *****************************************************************************/
static int
DeviceList_RemoveAll (void)
{
  EF_DBG_FUNC(1, NULL);
  DeviceList_Lock();
  EF_DBG_FUNC(2, NULL);

  ListNode* node;
  for (node = ListHead (&GlobalDeviceList);
    node != 0;
    node = ListNext (&GlobalDeviceList, node)) {
    DeviceNode* devnode = node->item;
    node->item = 0;
    // Do the notifications while the global list is still locked
    NotifyUpdate (E_DEVICE_REMOVED, devnode);
    talloc_free (devnode);
  }
  ListDestroy (&GlobalDeviceList, /*freeItem=>*/ 0);
  ListInit (&GlobalDeviceList, 0, 0);

  DeviceList_Unlock();

  return UPNP_E_SUCCESS;
}

int DeviceList_GetDeviceCount (void)
{
  int c = 0;
  DeviceList_Lock();
  c = ListSize(&GlobalDeviceList);
  DeviceList_Unlock();

  return c;
}


/*****************************************************************************
 * DeviceList_RefreshAll
 *****************************************************************************/
int
DeviceList_RefreshAll (bool remove_all)
{
  if (remove_all)
    (void) DeviceList_RemoveAll();

  /*
   * Search for all 'target' providers,
   * waiting for up to 5 seconds for the response
   */
  Log_Printf (LOG_DEBUG, "RefreshAll target=%s", NN(g_ssdp_target));
  int rc = UpnpSearchAsync (g_ctrlpt_handle, 3 /* seconds */,
    g_ssdp_target, NULL);
  if (UPNP_E_SUCCESS != rc)
    Log_Printf (LOG_ERROR, "Error sending search request %d", rc);

  return rc;
}


/*****************************************************************************
 * HandleEvent
 *
 * Description:
 *       Handle a UPnP event that was received.  Process the event and update
 *       the appropriate service state table.
 *
 * Parameters:
 *   sid -- The subscription id for the event
 *   eventkey -- The eventkey number for the event
 *   changes -- The DOM document representing the changes
 *
 *****************************************************************************/
static void
HandleEvent (Upnp_SID sid,
  int eventkey,
  IXML_Document* changes )
{
  DeviceList_Lock();

  Log_Printf (LOG_DEBUG, "Received Event: %d for SID %s", eventkey, NN(sid));
  Service* const serv = GetService (sid, FROM_SID);
  char *udn = GetUdnBySid(sid);
  if (serv && udn )
    Service_UpdateState(serv, changes, udn);

  DeviceList_Unlock();
}


/*****************************************************************************
 * AddDevice
 *
 * Description:
 *       If the device is not already included in the global device list,
 *       add it.  Otherwise, update its advertisement expiration timeout.
 *
 * Parameters:
 *   descLocation -- The location of the description document URL
 *   expires -- The expiration time for this advertisement
 *
 *****************************************************************************/


static void AddDevice (const char* deviceId, const char* descLocation, int expires)
{
  DeviceList_Lock();

  DeviceNode* devnode = NULL;
  ListNode* node = GetDeviceListNodeFromId (deviceId);
  if (node)
    devnode = node->item;

  if(devnode) {
    // The device is already there, so just update
    // the advertisement timeout field
    HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MANY, "old: %s\n", deviceId);
    devnode->expires = expires;
    DeviceList_Unlock();
    return;
  }
extern int g_dmscp_dms_total_limit;
  if(ListSize(&GlobalDeviceList) > g_dmscp_dms_total_limit) {
    // too many DMS, it must be attacked
    HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MANY, "too many dms: %s\n", deviceId);
    DeviceList_Unlock();
    return;
  }
  // Else create a new device

  // *unlock* before trying to download the Device Description
  // Document, which can take a long time in some error cases
  // (e.g. timeout if network problems)
  DeviceList_Unlock();

  HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MANY, "new: %s; %s\n", deviceId, descLocation);

  if (descLocation == NULL)
    return; // ---------->

  char* descDocText = NULL;
  char content_type [LINE_SIZE] = "";
  int rc = UpnpDownloadUrlItem (descLocation, &descDocText, content_type);

  HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MANY, "UpnpDownloadUrlItem: %d, %s\n", rc, content_type);

  if (rc != UPNP_E_SUCCESS) {
    Log_Printf (LOG_ERROR,
      "Error obtaining device description from "
      "url '%s' : %d (%s)", descLocation, rc,
      UpnpGetErrorMessage (rc));
    if (rc/100 == UPNP_E_NETWORK_ERROR/100) {
      Log_Printf (LOG_ERROR, "Check device network "
        "configuration (firewall ?)");
    }
    return; // ---------->
  }
  if (strncasecmp (content_type, "text/xml", 8)) {
    // "text/xml" is specified in UPnP Device Architecture
    // v1.0 -- however don't abort if incorrect because
    // some broken UPnP device send other MIME types
    // (e.g. application/octet-stream).
    Log_Printf (LOG_ERROR, "Device description at url '%s'"
      " has MIME '%s' instead of XML ! "
      "Trying to parse anyway ...",
      descLocation, content_type);
  }

  void* context = NULL; // TBD should be parent talloc TBD XXX
  devnode = talloc (context, DeviceNode);
  // Initialize fields to empty values
  *devnode = (struct _DeviceNode) { };
  devnode->d = Device_Create (devnode, g_ctrlpt_handle, descLocation, deviceId, descDocText);
  free (descDocText);
  descDocText = NULL;

  if (devnode->d == NULL) {
    Log_Printf (LOG_ERROR, "Can't create Device Id=%s",
      NN(deviceId));
    talloc_free (devnode);
    return; // ---------->
  } else {
    // If SSDP target specified, check that the device
    // matches it.
    if (strstr (g_ssdp_target, ":service:")) {
      const Service* serv = Device_GetServiceFrom(devnode->d, g_ssdp_target, FROM_SERVICE_TYPE, false);
      if (serv == NULL) {
        Log_Printf (LOG_DEBUG,
          "Discovered device Id=%s "
          "has no '%s' service : "
          "forgetting", NN(deviceId),
          g_ssdp_target);
        talloc_free (devnode);
        return; // ---------->
      }
    }

    // Relock the device list (and check that the same
    // device has not already been added by another thread
    // while the list was unlocked)
    DeviceList_Lock();
    node = GetDeviceListNodeFromId (deviceId);
    if (node) {
      Log_Printf (LOG_WARNING,
        "Device Id=%s already added",
        NN(deviceId));
      // Delete extraneous device descriptor. Note:
      // service subscription is not yet done, so
      // the Service destructors will not unsubscribe
      talloc_free (devnode);
    }
    else
    {
      devnode->deviceId = talloc_strdup (devnode, deviceId);
      devnode->expires = expires;

      // Generate a unique, friendly, name
      // for this device
      const char* base = Device_GetDescDocItem(devnode->d, "friendlyName", true);
      char* res = make_device_name (NULL, base);
      //char *name = talloc_asprintf (NULL, "%d %s", Device_get_dms_stamp(devnode->d), res);
      talloc_set_name (devnode->d, "%s", res);
      //talloc_free (name);
      talloc_free(res);

      // Insert the new device node in the list
      ListAddTail (&GlobalDeviceList, devnode);

      Device_SusbcribeAllEvents (devnode->d);

      // Notify New Device Added, while the global
      // list is still locked
      NotifyUpdate (E_DEVICE_ADDED, devnode);
    }
    DeviceList_Unlock();

  }
}



/*****************************************************************************
 * EventHandlerCallback
 *
 * Description:
 *       The callback handler registered with the SDK while registering
 *       the control point.  Detects the type of callback, and passes the
 *       request on to the appropriate function.
 *
 * Parameters:
 *   event_type -- The type of callback event
 *   event -- Data structure containing event data
 *   cookie -- Optional data specified during callback registration
 *
 *****************************************************************************/
/* the head files as below for inet_addr */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int
EventHandlerCallback (Upnp_EventType event_type,
  void* event, void* cookie)
{
  // Create a working context for temporary strings
  //	void* const tmp_ctx = talloc_new (NULL);

  //Log_Print (LOG_DEBUG, UpnpUtil_GetEventString (tmp_ctx, event_type, event));

  HT_DBG_FUNC_START(HT_MOD_DMC, HT_BIT_MYRIAD, event_type, NULL);
  switch ( event_type ) {
    /*
     * SSDP Stuff
     */
  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
  case UPNP_DISCOVERY_SEARCH_RESULT:
    {
      const struct Upnp_Discovery* const e =
        (struct Upnp_Discovery*) event;
      struct sockaddr_in* inaddr = &(e->DestAddr);
      if (e->ErrCode != UPNP_E_SUCCESS) {
        Log_Printf (LOG_ERROR,
          "Error in Discovery Callback -- %d",
          e->ErrCode);
      }
      // TBD else ??
      // char *pp=strrchr(e->DeviceId, '-');
      HT_DBG_PRINTF(HT_MOD_DMC, HT_BIT_MYRIAD, "from UDN: %s, DT: %s, ST: %s\n", strrchr(e->DeviceId, '-'), e->DeviceType, e->ServiceType);
      if( DMS_TYPE_MATCH(e->DeviceType) || CDS_TYPE_MATCH(e->ServiceType) )
        if (e->DeviceId && e->DeviceId[0]) {
          Log_Printf (LOG_DEBUG,
            "Discovery : device type '%s' "
            "OS '%s' at URL '%s'", NN(e->DeviceType),
            NN(e->Os), NN(e->Location));
          extern int g_dmscp_show_inner_dms;
          if((inet_addr(UpnpGetServerIpAddress()) != inaddr->sin_addr.s_addr) || g_dmscp_show_inner_dms)
            AddDevice (e->DeviceId, e->Location, e->Expires);
          //			Log_Printf (LOG_DEBUG, "Discovery: "
          //				    "DeviceList after AddDevice = \n%s",
          //				    DeviceList_GetStatusString (tmp_ctx));
        }

      break;
    }

  case UPNP_DISCOVERY_SEARCH_TIMEOUT:
    /*
     * Nothing to do here...
     */
    break;

  case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
    {
      struct Upnp_Discovery* e = (struct Upnp_Discovery*) event;

      if (e->ErrCode != UPNP_E_SUCCESS ) {
        Log_Printf (LOG_ERROR,
          "Error in Discovery ByeBye Callback -- %d",
          e->ErrCode );
      }

      Log_Printf (LOG_DEBUG, "Received ByeBye for Device: %s",
        e->DeviceId );
      DeviceList_RemoveDevice (e->DeviceId);

      //		Log_Printf (LOG_DEBUG, "DeviceList after byebye: \n%s",
      //			    DeviceList_GetStatusString (tmp_ctx));
      break;
    }

    /*
     * SOAP Stuff
     */
  case UPNP_CONTROL_ACTION_COMPLETE:
    {
      struct Upnp_Action_Complete* e =
        (struct Upnp_Action_Complete*) event;

      if (e->ErrCode != UPNP_E_SUCCESS ) {
        Log_Printf (LOG_ERROR,
          "Error in  Action Complete Callback -- %d",
          e->ErrCode );
      }

      /*
       * No need for any processing here, just print out results.
       * Service state table updates are handled by events.
       */

      break;
    }

  case UPNP_CONTROL_GET_VAR_COMPLETE:
    /*
     * Not used : deprecated
     */
    Log_Printf (LOG_WARNING,
      "Deprecated Get Var Complete Callback");
    break;

    /*
     * GENA Stuff
     */
  case UPNP_EVENT_RECEIVED:
    {
      struct Upnp_Event* e = (struct Upnp_Event*) event;

      HandleEvent (e->Sid, e->EventKey, e->ChangedVariables);
      break;
    }

  case UPNP_EVENT_SUBSCRIBE_COMPLETE:
  case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
  case UPNP_EVENT_RENEWAL_COMPLETE:
    {
      struct Upnp_Event_Subscribe* e =
        (struct Upnp_Event_Subscribe*) event;

      if (e->ErrCode != UPNP_E_SUCCESS ) {
        Log_Printf (LOG_ERROR,
          "Error in Event Subscribe Callback -- %d",
          e->ErrCode );
      } else {
        Log_Printf (LOG_DEBUG,
          "Received Event Renewal for eventURL %s",
          NN(e->PublisherUrl));

        DeviceList_Lock();

        Service* const serv = GetService (e->PublisherUrl,
          FROM_EVENT_URL);
        if (serv) {
          if (event_type ==
            UPNP_EVENT_UNSUBSCRIBE_COMPLETE)
            Service_SetSid (serv, NULL);
          else
            Service_SetSid (serv, e->Sid);
        }
        DeviceList_Unlock();
      }
      break;
    }

  case UPNP_EVENT_AUTORENEWAL_FAILED:
  case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
    {
      struct Upnp_Event_Subscribe* e =
        (struct Upnp_Event_Subscribe*) event;

      Log_Printf (LOG_DEBUG, "Renewing subscription for eventURL %s",
        NN(e->PublisherUrl));

      DeviceList_Lock();

      Service* const serv = GetService (e->PublisherUrl,
        FROM_EVENT_URL);
      if (serv)
        Service_SubscribeEventURL (serv);

      DeviceList_Unlock();

      break;
    }

    /*
     * ignore these cases, since this is not a device
     */
  case UPNP_EVENT_SUBSCRIPTION_REQUEST:
  case UPNP_CONTROL_GET_VAR_REQUEST:
  case UPNP_CONTROL_ACTION_REQUEST:
    break;
  }

  // Delete all temporary strings
  //	talloc_free (tmp_ctx);
  HT_DBG_FUNC_END(0, 0);
  return 0;
}

#if 0

#endif

/*****************************************************************************
 * VerifyTimeouts
 *
 * Description:
 *       Checks the advertisement  each device
 *        in the global device list.  If an advertisement expires,
 *       the device is removed from the list.  If an advertisement is about to
 *       expire, a search request is sent for that device.
 *
 * Parameters:
 *    incr -- The increment to subtract from the timeouts each time the
 *            function is called.
 *
 *****************************************************************************/
static void
VerifyTimeouts (int incr)
{
  DeviceList_Lock();

  // During this traversal we pre-compute the next node in case
  // the current node is deleted
  ListNode* node, *nextnode = 0;
  for (node = ListHead (&GlobalDeviceList); node != 0; node = nextnode) {
    nextnode = ListNext (&GlobalDeviceList, node);

    DeviceNode* devnode = node->item;
    devnode->expires -= incr;
    if (devnode->expires <= 0) {
      /*
       * This advertisement has expired, so we should remove the device
       * from the list
       */
      Log_Printf (LOG_DEBUG, "Remove expired device Id=%s", devnode->deviceId);
      node->item = 0;
      ListDelNode (&GlobalDeviceList, node, /*freeItem=>*/ 0);
      // Do the notification while the global list is locked
      NotifyUpdate (E_DEVICE_REMOVED, devnode);
      talloc_free (devnode);
    }
  }
  DeviceList_Unlock();
}

#if 0
#endif

#include <semaphore.h>

static int kill_pipe[2];
static sem_t killed_sem;


static void*
CheckSubscriptionsLoop (void* arg)
{
  fd_set rset;
  struct timeval tv;
  int ret;
  int sec = CHECK_SUBSCRIPTIONS_TIMEOUT;

  HT_DBG_FUNC_START(HT_MOD_UPNP,HT_BIT_MANY,0, NULL);
  while(1)
  {
    tv.tv_sec  = sec;// CHECK_SUBSCRIPTIONS_TIMEOUT;
    tv.tv_usec = 0;

    FD_ZERO(&rset);
    FD_SET( kill_pipe[0], &rset);

    ret = select(kill_pipe[0] + 1, &rset, NULL, NULL, &tv);
    HT_DBG_FUNC(ret, NULL);
    if (ret == 1)
    {
      break;
    }

    VerifyTimeouts(sec);
  }

  sem_post(&killed_sem);
  HT_DBG_FUNC_END(ret, NULL);

  return NULL;
}


/*****************************************************************************
 * DeviceList_Start
 *****************************************************************************/
int
DeviceList_init (DeviceList_EventCallback eventCallback, ServiceUpdatedCallback serviceUpdatedCallback)
{

  char target[NAME_SIZE];

  sprintf(target,"%s%d", CDS_TYPE_BASE, CDS_TYPE_VERSION);
  g_ssdp_target = talloc_strdup (NULL, target);

  gStateUpdateFun = eventCallback;
  gServiceUpdateCallback = serviceUpdatedCallback;

  ithread_mutex_init (&DeviceListMutex, NULL);

  ListInit (&GlobalDeviceList, 0, 0);

  sem_init(&killed_sem,0,0);
  pipe(kill_pipe);
  return 0;
}

int
DeviceList_register_ctrlpt(void)
{
  // Makes the XML parser more tolerant to malformed text
  ixmlRelaxParser ('?');
  UpnpRegisterClient (EventHandlerCallback, &g_ctrlpt_handle, &g_ctrlpt_handle);
  DeviceList_RefreshAll (true);

  // start a timer thread
  ithread_create (&g_timer_thread, NULL, CheckSubscriptionsLoop, NULL);
  return 0;
}


int
DeviceList_unregister_ctrlpt (void)
{
  char buf[8];
  int ret;

  ret = write(kill_pipe[1], buf, 1);
  EF_DBG_FUNC(ret, NULL);
  sem_wait(&killed_sem);
  EF_DBG_FUNC(0, NULL);
  ret = read(kill_pipe[0], buf, sizeof(buf));
  EF_DBG_FUNC(ret, NULL);

  DeviceList_RemoveAll();
  UpnpUnRegisterClient (g_ctrlpt_handle);
  return 0;
}



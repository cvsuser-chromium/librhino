///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2003 Intel Corporation
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// * Neither name of Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#include "upnp.h"
#include "ithread.h"
#include "upnp_util_dmr.h"
#include <stdio.h>
#include <stdarg.h>

#if !UPNP_HAVE_TOOLS
#	error "Need upnptools.h to compile Upnps ; try ./configure --enable-tools"
#endif

/*
   Function pointer to use for displaying formatted
   strings. Set on Initialization of device.
 */
static int initialize = 1;
static print_string gPrintFun = NULL;
static state_update gStateUpdateFun = NULL;

//mutex to control displaying of events
static ithread_mutex_t display_mutex;

/********************************************************************************
 * UpnpUtil_Initialize
 *
 * Description:
 *     Initializes the Upnp util. Must be called before any Upnp util
 *     functions. May be called multiple times.
 *
 * Parameters:
 *   print_function - print function to use in UpnpUtil_Print
 *
 ********************************************************************************/
int
UpnpUtil_Initialize( print_string print_function )
{
    if( initialize ) {
        ithread_mutexattr_t attr;

        gPrintFun = print_function;
        ithread_mutexattr_init( &attr );
        ithread_mutexattr_setkind_np( &attr, ITHREAD_MUTEX_RECURSIVE_NP );
        ithread_mutex_init( &display_mutex, &attr );
        ithread_mutexattr_destroy( &attr );
        initialize = 0;
    }
    return UPNP_E_SUCCESS;
}

#if 0
/********************************************************************************
 * UpnpUtil_RegisterUpdateFunction
 *
 * Description:
 *
 * Parameters:
 *
 ********************************************************************************/
int
UpnpUtil_RegisterUpdateFunction( state_update update_function )
{
    static int initialize = 1;  //only intialize once

    if( initialize ) {
        gStateUpdateFun = update_function;
        initialize = 0;
    }
    return UPNP_E_SUCCESS;
}
#endif
/********************************************************************************
 * UpnpUtil_Finish
 *
 * Description:
 *     Releases Resources held by Upnp util.
 *
 * Parameters:
 *
 ********************************************************************************/
int
UpnpUtil_Finish(void)
{
    ithread_mutex_destroy( &display_mutex );
    gPrintFun = NULL;
    initialize = 1;
    return UPNP_E_SUCCESS;
}
/********************************************************************************
 * UpnpUtil_GetElementValue
 *
 * Description:
 *       Given a DOM node such as <Channel>11</Channel>, this routine
 *       extracts the value (e.g., 11) from the node and returns it as
 *       a string. The string must be freed by the caller using
 *       free.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the value
 *
 ********************************************************************************/

char *
UpnpUtil_GetElementValue( IN IXML_Element * element )
{

    IXML_Node *child = ixmlNode_getFirstChild( ( IXML_Node * ) element );

    char *temp = NULL;

    if( ( child != 0 ) && ( ixmlNode_getNodeType( child ) == eTEXT_NODE ) ) {
        temp = strdup( ixmlNode_getNodeValue( child ) );
    }

    return temp;
}

/********************************************************************************
 * UpnpUtil_GetFirstServiceList
 *
 * Description:
 *       Given a DOM node representing a UPnP Device Description Document,
 *       this routine parses the document and finds the first service list
 *       (i.e., the service list for the root device).  The service list
 *       is returned as a DOM node list.
 *
 * Parameters:
 *   node -- The DOM node from which to extract the service list
 *
 ********************************************************************************/
IXML_NodeList *
UpnpUtil_GetFirstServiceList( IN IXML_Document * doc )
{
    IXML_NodeList *ServiceList = NULL;
    IXML_NodeList *servlistnodelist = NULL;
    IXML_Node *servlistnode = NULL;

    servlistnodelist =
        ixmlDocument_getElementsByTagName( doc, "serviceList" );
    if( servlistnodelist && ixmlNodeList_length( servlistnodelist ) ) {

        /*
           we only care about the first service list, from the root device
         */
        servlistnode = ixmlNodeList_item( servlistnodelist, 0 );

        /*
           create as list of DOM nodes
         */
        ServiceList =
            ixmlElement_getElementsByTagName( ( IXML_Element * )
                                              servlistnode, "service" );
    }

    if( servlistnodelist )
        ixmlNodeList_free( servlistnodelist );

    return ServiceList;
}

/********************************************************************************
 * UpnpUtil_GetFirstDocumentItem
 *
 * Description:
 *       Given a document node, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       String must be freed by caller using free.
 * Parameters:
 *   doc -- The DOM document from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char *
UpnpUtil_GetFirstDocumentItem( IN IXML_Document * doc,
                                 IN const char *item )
{
  IXML_NodeList *nodeList = NULL;
  IXML_Node *textNode = NULL;
  IXML_Node *tmpNode = NULL;

  char *ret = NULL;

  nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );

  if( nodeList ) {
    if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {
      textNode = ixmlNode_getFirstChild( tmpNode );
      if(ixmlNode_getNodeValue( textNode ))
        ret = strdup( ixmlNode_getNodeValue( textNode ) );
    }
  }

  if( nodeList )
    ixmlNodeList_free( nodeList );
  return ret;
}

/********************************************************************************
 * UpnpUtil_SetFirstDocumentItem
 *
 * Description:
 *       Given a document node, this routine searches for the first element
 *       named by the input string item, and set its value .
 * Parameters:
 *   doc -- The DOM document from which to extract the value
 *   item -- The item to search for
 *   value -- The itme new value
 *
 ********************************************************************************/
int
UpnpUtil_SetFirstDocumentItem( IN IXML_Document * doc,
                                 IN const char *item, char *value )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    int ret = -1;

    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );

    if( nodeList ) {
        if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {
            textNode = ixmlNode_getFirstChild( tmpNode );
	     ret = ixmlNode_setNodeValue(textNode, value);
        }
    }
    if( nodeList )
        ixmlNodeList_free( nodeList );
    return ret;
}


/********************************************************************************
 * UpnpUtil_GetFirstElementItem
 *
 * Description:
 *       Given a DOM element, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       The string must be freed using free.
 * Parameters:
 *   node -- The DOM element from which to extract the value
 *   item -- The item to search for
 *
 ********************************************************************************/
char *
UpnpUtil_GetFirstElementItem( IN IXML_Element * element,
                                IN const char *item )
{
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;

    nodeList = ixmlElement_getElementsByTagName( element, ( char * )item );

    if( nodeList == NULL ) {
        UpnpUtil_Print( "Error finding %s in XML Node\n", item );
        return NULL;
    }

    if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) == NULL ) {
        UpnpUtil_Print( "Error finding %s value in XML Node\n", item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }

    textNode = ixmlNode_getFirstChild( tmpNode );

    ret = strdup( ixmlNode_getNodeValue( textNode ) );

    if( !ret ) {
        UpnpUtil_Print( "Error allocating memory for %s in XML Node\n",
                          item );
        ixmlNodeList_free( nodeList );
        return NULL;
    }

    ixmlNodeList_free( nodeList );

    return ret;
}
/********************************************************************************
 * UpnpUtil_PrintEventType
 *
 * Description:
 *       Prints a callback event type as a string.
 *
 * Parameters:
 *   S -- The callback event
 *
 ********************************************************************************/
void
UpnpUtil_PrintEventType( IN Upnp_EventType S )
{
    switch ( S ) {

        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
            UpnpUtil_Print( "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE\n" );
            break;
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
            UpnpUtil_Print( "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE\n" );
            break;
        case UPNP_DISCOVERY_SEARCH_RESULT:
            UpnpUtil_Print( "UPNP_DISCOVERY_SEARCH_RESULT\n" );
            break;
        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            UpnpUtil_Print( "UPNP_DISCOVERY_SEARCH_TIMEOUT\n" );
            break;

            /*
               SOAP Stuff
             */
        case UPNP_CONTROL_ACTION_REQUEST:
            UpnpUtil_Print( "UPNP_CONTROL_ACTION_REQUEST\n" );
            break;
        case UPNP_CONTROL_ACTION_COMPLETE:
            UpnpUtil_Print( "UPNP_CONTROL_ACTION_COMPLETE\n" );
            break;
        case UPNP_CONTROL_GET_VAR_REQUEST:
            UpnpUtil_Print( "UPNP_CONTROL_GET_VAR_REQUEST\n" );
            break;
        case UPNP_CONTROL_GET_VAR_COMPLETE:
            UpnpUtil_Print( "UPNP_CONTROL_GET_VAR_COMPLETE\n" );
            break;

            /*
               GENA Stuff
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            UpnpUtil_Print( "UPNP_EVENT_SUBSCRIPTION_REQUEST\n" );
            break;
        case UPNP_EVENT_RECEIVED:
            UpnpUtil_Print( "UPNP_EVENT_RECEIVED\n" );
            break;
        case UPNP_EVENT_RENEWAL_COMPLETE:
            UpnpUtil_Print( "UPNP_EVENT_RENEWAL_COMPLETE\n" );
            break;
        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
            UpnpUtil_Print( "UPNP_EVENT_SUBSCRIBE_COMPLETE\n" );
            break;
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            UpnpUtil_Print( "UPNP_EVENT_UNSUBSCRIBE_COMPLETE\n" );
            break;

        case UPNP_EVENT_AUTORENEWAL_FAILED:
            UpnpUtil_Print( "UPNP_EVENT_AUTORENEWAL_FAILED\n" );
            break;
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            UpnpUtil_Print( "UPNP_EVENT_SUBSCRIPTION_EXPIRED\n" );
            break;

    }
}

/********************************************************************************
 * UpnpUtil_PrintEvent
 *
 * Description:
 *       Prints callback event structure details.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- The callback event structure
 *
 ********************************************************************************/
int
UpnpUtil_PrintEvent( IN Upnp_EventType EventType,
                       IN void *Event )
{

    ithread_mutex_lock( &display_mutex );

    UpnpUtil_Print
        ( "\n\n\n======================================================================\n" );
    UpnpUtil_Print
        ( "----------------------------------------------------------------------\n" );
    UpnpUtil_PrintEventType( EventType );

    switch ( EventType ) {

            /*
               SSDP
             */
        case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
        case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
        case UPNP_DISCOVERY_SEARCH_RESULT:
            {
                struct Upnp_Discovery *d_event =
                    ( struct Upnp_Discovery * )Event;

                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  d_event->ErrCode );
                UpnpUtil_Print( "Expires     =  %d\n",
                                  d_event->Expires );
                UpnpUtil_Print( "DeviceId    =  %s\n",
                                  d_event->DeviceId );
                UpnpUtil_Print( "DeviceType  =  %s\n",
                                  d_event->DeviceType );
                UpnpUtil_Print( "ServiceType =  %s\n",
                                  d_event->ServiceType );
                UpnpUtil_Print( "ServiceVer  =  %s\n",
                                  d_event->ServiceVer );
                UpnpUtil_Print( "Location    =  %s\n",
                                  d_event->Location );
                UpnpUtil_Print( "OS          =  %s\n", d_event->Os );
                UpnpUtil_Print( "Ext         =  %s\n", d_event->Ext );

            }
            break;

        case UPNP_DISCOVERY_SEARCH_TIMEOUT:
            // Nothing to print out here
            break;

            /*
               SOAP
             */
        case UPNP_CONTROL_ACTION_REQUEST:
            {
                struct Upnp_Action_Request *a_event =
                    ( struct Upnp_Action_Request * )Event;
                char *xmlbuff = NULL;

                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  a_event->ErrCode );
                UpnpUtil_Print( "ErrStr      =  %s\n", a_event->ErrStr );
                UpnpUtil_Print( "ActionName  =  %s\n",
                                  a_event->ActionName );
                UpnpUtil_Print( "UDN         =  %s\n", a_event->DevUDN );
                UpnpUtil_Print( "ServiceID   =  %s\n",
                                  a_event->ServiceID );
                if( a_event->ActionRequest ) {
                    xmlbuff = ixmlPrintNode( ( IXML_Node * ) a_event->ActionRequest );
                    if( xmlbuff )
                        UpnpUtil_Print( "ActRequest  =  %s\n", xmlbuff );
                    if( xmlbuff )
                        ixmlFreeDOMString( xmlbuff );
                    xmlbuff = NULL;
                } else {
                    UpnpUtil_Print( "ActRequest  =  (null)\n" );
                }

                if( a_event->ActionResult ) {
                    xmlbuff = ixmlPrintNode( ( IXML_Node * ) a_event->ActionResult );
                    if( xmlbuff )
                        UpnpUtil_Print( "ActResult   =  %s\n", xmlbuff );
                    if( xmlbuff )
                        ixmlFreeDOMString( xmlbuff );
                    xmlbuff = NULL;
                } else {
                    UpnpUtil_Print( "ActResult   =  (null)\n" );
                }
            }
            break;

        case UPNP_CONTROL_ACTION_COMPLETE:
            {
                struct Upnp_Action_Complete *a_event =
                    ( struct Upnp_Action_Complete * )Event;
                char *xmlbuff = NULL;

                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  a_event->ErrCode );
                UpnpUtil_Print( "CtrlUrl     =  %s\n",
                                  a_event->CtrlUrl );
                if( a_event->ActionRequest ) {
                    xmlbuff = ixmlPrintNode( ( IXML_Node * ) a_event->ActionRequest );
                    if( xmlbuff )
                        UpnpUtil_Print( "ActRequest  =  %s\n", xmlbuff );
                    if( xmlbuff )
                        ixmlFreeDOMString( xmlbuff );
                    xmlbuff = NULL;
                } else {
                    UpnpUtil_Print( "ActRequest  =  (null)\n" );
                }

                if( a_event->ActionResult ) {
                    xmlbuff = ixmlPrintNode( ( IXML_Node * ) a_event->ActionResult );
                    if( xmlbuff )
                        UpnpUtil_Print( "ActResult   =  %s\n", xmlbuff );
                    if( xmlbuff )
                        ixmlFreeDOMString( xmlbuff );
                    xmlbuff = NULL;
                } else {
                    UpnpUtil_Print( "ActResult   =  (null)\n" );
                }
            }
            break;

        case UPNP_CONTROL_GET_VAR_REQUEST:
            {
                struct Upnp_State_Var_Request *sv_event =
                    ( struct Upnp_State_Var_Request * )Event;

                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  sv_event->ErrCode );
                UpnpUtil_Print( "ErrStr      =  %s\n",
                                  sv_event->ErrStr );
                UpnpUtil_Print( "UDN         =  %s\n",
                                  sv_event->DevUDN );
                UpnpUtil_Print( "ServiceID   =  %s\n",
                                  sv_event->ServiceID );
                UpnpUtil_Print( "StateVarName=  %s\n",
                                  sv_event->StateVarName );
                UpnpUtil_Print( "CurrentVal  =  %s\n",
                                  sv_event->CurrentVal );
            }
            break;

        case UPNP_CONTROL_GET_VAR_COMPLETE:
            {
                struct Upnp_State_Var_Complete *sv_event =
                    ( struct Upnp_State_Var_Complete * )Event;

                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  sv_event->ErrCode );
                UpnpUtil_Print( "CtrlUrl     =  %s\n",
                                  sv_event->CtrlUrl );
                UpnpUtil_Print( "StateVarName=  %s\n",
                                  sv_event->StateVarName );
                UpnpUtil_Print( "CurrentVal  =  %s\n",
                                  sv_event->CurrentVal );
            }
            break;

            /*
               GENA
             */
        case UPNP_EVENT_SUBSCRIPTION_REQUEST:
            {
                struct Upnp_Subscription_Request *sr_event =
                    ( struct Upnp_Subscription_Request * )Event;

                UpnpUtil_Print( "ServiceID   =  %s\n",
                                  sr_event->ServiceId );
                UpnpUtil_Print( "UDN         =  %s\n", sr_event->UDN );
                UpnpUtil_Print( "SID         =  %s\n", sr_event->Sid );
            }
            break;

        case UPNP_EVENT_RECEIVED:
            {
                struct Upnp_Event *e_event = ( struct Upnp_Event * )Event;
                char *xmlbuff = NULL;

                UpnpUtil_Print( "SID         =  %s\n", e_event->Sid );
                UpnpUtil_Print( "EventKey    =  %d\n",
                                  e_event->EventKey );
                xmlbuff = ixmlPrintNode( ( IXML_Node * ) e_event->ChangedVariables );
                UpnpUtil_Print( "ChangedVars =  %s\n", xmlbuff );
                ixmlFreeDOMString( xmlbuff );
                xmlbuff = NULL;
            }
            break;

        case UPNP_EVENT_RENEWAL_COMPLETE:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                UpnpUtil_Print( "SID         =  %s\n", es_event->Sid );
                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  es_event->ErrCode );
                UpnpUtil_Print( "TimeOut     =  %d\n",
                                  es_event->TimeOut );
            }
            break;

        case UPNP_EVENT_SUBSCRIBE_COMPLETE:
        case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                UpnpUtil_Print( "SID         =  %s\n", es_event->Sid );
                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  es_event->ErrCode );
                UpnpUtil_Print( "PublisherURL=  %s\n",
                                  es_event->PublisherUrl );
                UpnpUtil_Print( "TimeOut     =  %d\n",
                                  es_event->TimeOut );
            }
            break;

        case UPNP_EVENT_AUTORENEWAL_FAILED:
        case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
            {
                struct Upnp_Event_Subscribe *es_event =
                    ( struct Upnp_Event_Subscribe * )Event;

                UpnpUtil_Print( "SID         =  %s\n", es_event->Sid );
                UpnpUtil_Print( "ErrCode     =  %d\n",
                                  es_event->ErrCode );
                UpnpUtil_Print( "PublisherURL=  %s\n",
                                  es_event->PublisherUrl );
                UpnpUtil_Print( "TimeOut     =  %d\n",
                                  es_event->TimeOut );
            }
            break;

    }
    UpnpUtil_Print
        ( "----------------------------------------------------------------------\n" );
    UpnpUtil_Print
        ( "======================================================================\n\n\n\n" );

    ithread_mutex_unlock( &display_mutex );
    return ( 0 );
}
/********************************************************************************
 * UpnpUtil_FindAndParseService
 *
 * Description:
 *       This routine finds the first occurance of a service in a DOM representation
 *       of a description document and parses it.
 *
 * Parameters:
 *   DescDoc -- The DOM description document
 *   location -- The location of the description document
 *   serviceSearchType -- The type of service to search for
 *   serviceId -- OUT -- The service ID
 *   eventURL -- OUT -- The event URL for the service
 *   controlURL -- OUT -- The control URL for the service
 *
 ********************************************************************************/
int
UpnpUtil_FindAndParseService( IN IXML_Document * DescDoc,
                                IN const char *location,
                                IN const char *serviceType,
                                OUT char **serviceId,
                                OUT char **eventURL,
                                OUT char **controlURL )
{
    int i,
      length,
      found = 0;
    int ret;
    char *tempServiceType = NULL;
    char *baseURL = NULL;
    const char *base = NULL;
    char *relcontrolURL = NULL,
     *releventURL = NULL;
    IXML_NodeList *serviceList = NULL;
    IXML_Element *service = NULL;

    baseURL = UpnpUtil_GetFirstDocumentItem( DescDoc, "URLBase" );

    if( baseURL )
        base = baseURL;
    else
        base = location;

    serviceList = UpnpUtil_GetFirstServiceList( DescDoc );
    length = ixmlNodeList_length( serviceList );
    for( i = 0; i < length; i++ ) {
        service = ( IXML_Element * ) ixmlNodeList_item( serviceList, i );
        tempServiceType =
            UpnpUtil_GetFirstElementItem( ( IXML_Element * ) service,
                                            "serviceType" );

        if( strcmp( tempServiceType, serviceType ) == 0 ) {
            UpnpUtil_Print( "Found service: %s\n", serviceType );
            *serviceId =
                UpnpUtil_GetFirstElementItem( service, "serviceId" );
            UpnpUtil_Print( "serviceId: %s\n", ( *serviceId ) );
            relcontrolURL =
                UpnpUtil_GetFirstElementItem( service, "controlURL" );
            releventURL =
                UpnpUtil_GetFirstElementItem( service, "eventSubURL" );

            *controlURL =
                malloc( strlen( base ) + strlen( relcontrolURL ) + 1 );
            if( *controlURL ) {
                ret = UpnpResolveURL( base, relcontrolURL, *controlURL );
                if( ret != UPNP_E_SUCCESS )
                    UpnpUtil_Print
                        ( "Error generating controlURL from %s + %s\n",
                          base, relcontrolURL );
            }

            *eventURL =
                malloc( strlen( base ) + strlen( releventURL ) + 1 );
            if( *eventURL ) {
                ret = UpnpResolveURL( base, releventURL, *eventURL );
                if( ret != UPNP_E_SUCCESS )
                    UpnpUtil_Print
                        ( "Error generating eventURL from %s + %s\n", base,
                          releventURL );
            }

            if( relcontrolURL )
                free( relcontrolURL );
            if( releventURL )
                free( releventURL );
            relcontrolURL = releventURL = NULL;

            found = 1;
            break;
        }

        if( tempServiceType )
            free( tempServiceType );
        tempServiceType = NULL;
    }

    if( tempServiceType )
        free( tempServiceType );
    if( serviceList )
        ixmlNodeList_free( serviceList );
    if( baseURL )
        free( baseURL );

    return ( found );
}
#if 0
// printf wrapper for portable code
int
uprint1( char *fmt,
         ... )
{
    va_list ap;
    char *buf = NULL;
    int size;

    va_start( ap, fmt );
    size = vsnprintf( buf, 0, fmt, ap );
    va_end( ap );

    if( size > 0 ) {
        buf = ( char * )malloc( size + 1 );
        if( vsnprintf( buf, size + 1, fmt, ap ) != size ) {
            free( buf );
            buf = NULL;
        }
    }

    if( buf ) {
        ithread_mutex_lock( &display_mutex );
        if( gPrintFun )
            gPrintFun( buf );
        ithread_mutex_unlock( &display_mutex );
        free( buf );
    }

    return size;
}
#endif
/********************************************************************************
 * UpnpUtil_Print
 *
 * Description:
 *      Provides platform-specific print functionality.  This function should be
 *      called when you want to print content suitable for console output (i.e.,
 *      in a large text box or on a screen).  If your device/operating system is
 *      not supported here, you should add a port.
 *
 * Parameters:
 *		Same as printf()
 *
 ********************************************************************************/
int
UpnpUtil_Print( const char *fmt,
                  ... )
{
    va_list ap;
    char buf[200];
    int rc;

    va_start( ap, fmt );
    rc = vsnprintf( buf, 200, fmt, ap );
    va_end( ap );

    ithread_mutex_lock( &display_mutex );
    if( gPrintFun )
        gPrintFun( buf );
    ithread_mutex_unlock( &display_mutex );

    return rc;
}
#if 0
/********************************************************************************
 * UpnpUtil_StateUpdate
 *
 * Description:
 *
 * Parameters:
 *
 ********************************************************************************/
void
UpnpUtil_StateUpdate( const char *varName,
                        const char *varValue,
                        const char *UDN,
                        eventType type )
{
    if( gStateUpdateFun )
        gStateUpdateFun( varName, varValue, UDN, type );
    // TBD: Add mutex here?
}
#endif


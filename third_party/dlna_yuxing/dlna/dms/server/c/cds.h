/*
 * cds.h : GeeXboX uShare Content Directory Service header.
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

#ifndef CDS_H_
#define CDS_H_

#define CDS_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">" \
"  <specVersion>" \
"    <major>1</major>" \
"    <minor>0</minor>" \
"  </specVersion>" \
"  <actionList>" \
"    <action>" \
"      <name>Browse</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>ObjectID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>BrowseFlag</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_BrowseFlag</relatedStateVariable>" \
"       </argument>" \
"        <argument>" \
"          <name>Filter</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Filter</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>StartingIndex</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Index</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>RequestedCount</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>SortCriteria</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_SortCriteria</relatedStateVariable>" \
"       </argument>" \
"        <argument>" \
"          <name>Result</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Result</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>NumberReturned</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>TotalMatches</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>UpdateID</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_UpdateID</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"	 <action>" \
"	   <name>Search</name>" \
"	   <argumentList>" \
"		 <argument>" \
"		   <name>ContainerID</name>" \
"		   <direction>in</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>SearchCriteria</name>" \
"		   <direction>in</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_SearchCriteria</relatedStateVariable>" \
"		</argument>" \
"		 <argument>" \
"		   <name>Filter</name>" \
"		   <direction>in</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_Filter</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>StartingIndex</name>" \
"		   <direction>in</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_Index</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>RequestedCount</name>" \
"		   <direction>in</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>SortCriteria</name>" \
"		   <direction>in</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_SortCriteria</relatedStateVariable>" \
"		</argument>" \
"		 <argument>" \
"		   <name>Result</name>" \
"		   <direction>out</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_Result</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>NumberReturned</name>" \
"		   <direction>out</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>TotalMatches</name>" \
"		   <direction>out</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_Count</relatedStateVariable>" \
"		 </argument>" \
"		 <argument>" \
"		   <name>UpdateID</name>" \
"		   <direction>out</direction>" \
"		   <relatedStateVariable>A_ARG_TYPE_UpdateID</relatedStateVariable>" \
"		 </argument>" \
"	   </argumentList>" \
"	 </action>" \
"    <action>" \
"      <name>DestroyObject</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>ObjectID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetSystemUpdateID</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>Id</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>SystemUpdateID</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetSearchCapabilities</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>SearchCaps</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>SearchCapabilities</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>GetSortCapabilities</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>SortCaps</name>" \
"          <direction>out</direction>" \
"          <relatedStateVariable>SortCapabilities</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"    <action>" \
"      <name>UpdateObject</name>" \
"      <argumentList>" \
"        <argument>" \
"          <name>ObjectID</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_ObjectID</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>CurrentTagValue</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_TagValueList</relatedStateVariable>" \
"        </argument>" \
"        <argument>" \
"          <name>NewTagValue</name>" \
"          <direction>in</direction>" \
"          <relatedStateVariable>A_ARG_TYPE_TagValueList</relatedStateVariable>" \
"        </argument>" \
"      </argumentList>" \
"    </action>" \
"  </actionList>" \
"  <serviceStateTable>" \
"	 <stateVariable sendEvents=\"no\">" \
"	   <name>A_ARG_TYPE_SearchCriteria</name>" \
"	   <dataType>string</dataType>" \
"	 </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_BrowseFlag</name>" \
"      <dataType>string</dataType>" \
"      <allowedValueList>" \
"        <allowedValue>BrowseMetadata</allowedValue>" \
"        <allowedValue>BrowseDirectChildren</allowedValue>" \
"      </allowedValueList>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"yes\">" \
"      <name>SystemUpdateID</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Count</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_SortCriteria</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>SortCapabilities</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Index</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_ObjectID</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_UpdateID</name>" \
"      <dataType>ui4</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_TagValueList</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Result</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"   <stateVariable sendEvents=\"no\">" \
"      <name>SearchCapabilities</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"    <stateVariable sendEvents=\"no\">" \
"      <name>A_ARG_TYPE_Filter</name>" \
"      <dataType>string</dataType>" \
"    </stateVariable>" \
"  </serviceStateTable>" \
"</scpd>"

#define CDS_DESCRIPTION_LEN strlen (CDS_DESCRIPTION)

#define CDS_LOCATION "/web/cds.xml"

#define CDS_SERVICE_ID "urn:upnp-org:serviceId:ContentDirectory"
#define CDS_SERVICE_TYPE "urn:schemas-upnp-org:service:ContentDirectory:1"

/* Represent the CDS GetSearchCapabilities action. */
#define SERVICE_CDS_ACTION_SEARCH_CAPS "GetSearchCapabilities"

/* Represent the CDS GetSortCapabilities action. */
#define SERVICE_CDS_ACTION_SORT_CAPS "GetSortCapabilities"

/* Represent the CDS GetSystemUpdateID action. */
#define SERVICE_CDS_ACTION_UPDATE_ID "GetSystemUpdateID"

/* Represent the CDS Browse action. */
#define SERVICE_CDS_ACTION_BROWSE "Browse"

/* Represent the CDS Search action. */
#define SERVICE_CDS_ACTION_SEARCH "Search"

/* Represent the CDS SearchCaps argument. */
#define SERVICE_CDS_ARG_SEARCH_CAPS "SearchCaps"

/* Represent the CDS SortCaps argument. */
#define SERVICE_CDS_ARG_SORT_CAPS "SortCaps"

/* Represent the CDS UpdateId argument. */
#define SERVICE_CDS_ARG_UPDATE_ID "Id"

/* Represent the CDS StartingIndex argument. */
#define SERVICE_CDS_ARG_START_INDEX "StartingIndex"

/* Represent the CDS RequestedCount argument. */
#define SERVICE_CDS_ARG_REQUEST_COUNT "RequestedCount"

/* Represent the CDS ObjectID argument. */
#define SERVICE_CDS_ARG_OBJECT_ID "ObjectID"

/* Represent the CDS Filter argument. */
#define SERVICE_CDS_ARG_FILTER "Filter"

/* Represent the CDS BrowseFlag argument. */
#define SERVICE_CDS_ARG_BROWSE_FLAG "BrowseFlag"

/* Represent the CDS SortCriteria argument. */
#define SERVICE_CDS_ARG_SORT_CRIT "SortCriteria"

/* Represent the CDS SearchCriteria argument. */
#define SERVICE_CDS_ARG_SEARCH_CRIT "SearchCriteria"

/* Represent the CDS Root Object ID argument. */
#define SERVICE_CDS_ROOT_OBJECT_ID "0"

/* Represent the CDS DIDL Message Metadata Browse flag argument. */
#define SERVICE_CDS_BROWSE_METADATA "BrowseMetadata"

/* Represent the CDS DIDL Message DirectChildren Browse flag argument. */
#define SERVICE_CDS_BROWSE_CHILDREN "BrowseDirectChildren"

/* Represent the CDS DIDL Message Result argument. */
#define SERVICE_CDS_DIDL_RESULT "Result"

/* Represent the CDS DIDL Message NumberReturned argument. */
#define SERVICE_CDS_DIDL_NUM_RETURNED "NumberReturned"

/* Represent the CDS DIDL Message TotalMatches argument. */
#define SERVICE_CDS_DIDL_TOTAL_MATCH "TotalMatches"

/* Represent the CDS DIDL Message UpdateID argument. */
#define SERVICE_CDS_DIDL_UPDATE_ID "UpdateID"

/* DIDL parameters */
/* Represent the CDS DIDL Message Header Namespace. */
#define DIDL_NAMESPACE \
    "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" " \
    "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" " \
    "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\""

/* Represent the CDS DIDL Message Header Tag. */
#define DIDL_LITE "DIDL-Lite"
#if 0
/* Represent the CDS DIDL Message Item value. */
#define DIDL_ITEM "item"

/* Represent the CDS DIDL Message Item ID value. */
#define DIDL_ITEM_ID "id"

/* Represent the CDS DIDL Message Item Parent ID value. */
#define DIDL_ITEM_PARENT_ID "parentID"

/* Represent the CDS DIDL Message Item Restricted value. */
#define DIDL_ITEM_RESTRICTED "restricted"

/* Represent the CDS DIDL Message Item UPnP Class value. */
#define DIDL_ITEM_CLASS "upnp:class"

/* Represent the CDS DIDL Message Item Title value. */
#define DIDL_ITEM_TITLE "dc:title"

/* Represent the CDS DIDL Message Item Resource value. */
#define DIDL_RES "res"
/* Represent the CDS DIDL Message Item Protocol Info value. */
#define DIDL_RES_INFO "protocolInfo"
/* Represent the CDS DIDL Message Item Resource Size value. */
#define DIDL_RES_SIZE "size"
/* Represent the CDS DIDL Message Item Resource bitrate value. */
#define DIDL_RES_BITRATE "bitrate"
/* Represent the CDS DIDL Message Item Resource duration value. */
#define DIDL_RES_DURATION "duration"
/* Represent the CDS DIDL Message Item Resource resolution value. */
#define DIDL_RES_RESOLUTION "resolution"

/* Represent the CDS DIDL Message Container value. */
#define DIDL_CONTAINER "container"

/* Represent the CDS DIDL Message Container ID value. */
#define DIDL_CONTAINER_ID "id"

/* Represent the CDS DIDL Message Container Parent ID value. */
#define DIDL_CONTAINER_PARENT_ID "parentID"

/* Represent the CDS DIDL Message Container number of children value. */
#define DIDL_CONTAINER_CHILDS "childCount"

/* Represent the CDS DIDL Message Container Restricted value. */
#define DIDL_CONTAINER_RESTRICTED "restricted"

/* Represent the CDS DIDL Message Container Searchable value. */
#define DIDL_CONTAINER_SEARCH "searchable"

/* Represent the CDS DIDL Message Container UPnP Class value. */
#define DIDL_CONTAINER_CLASS "upnp:class"

/* Represent the CDS DIDL Message Container Title value. */
#define DIDL_CONTAINER_TITLE "dc:title"

/* Represent the "upnp:class" reserved keyword for Search action */
#define SEARCH_CLASS_MATCH_KEYWORD "(upnp:class = \""

/* Represent the "upnp:class derived from" reserved keyword */
#define SEARCH_CLASS_DERIVED_KEYWORD "(upnp:class derivedfrom \""

/* Represent the "res@protocolInfo contains" reserved keyword */
#define SEARCH_PROTOCOL_CONTAINS_KEYWORD "(res@protocolInfo contains \""

/* Represent the "res@protocolInfo contains" reserved keyword */
#define SEARCH_REFID_KEYWORD "(@refID exists "

/* Represent the Search default keyword */
#define SEARCH_OBJECT_KEYWORD "object"

/* Represent the Search 'AND' connector keyword */
#define SEARCH_AND " and "

/* Represent the Search 'parentID' reserved keyword */
#define SEARCH_PARENT_ID "(parentID = \""
#endif
extern int object_id_to_idx(char* object_id);

#endif /* CDS_H_ */


#ifndef AVRENDERSERVICE_H
#define AVRENDERSERVICE_H
char *av_render_service = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<root xmlns=\"urn:schemas-upnp-org:device-1-0\" xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">\r\n\
<specVersion>\r\n\
<major>1</major>\r\n\
<minor>0</minor>\r\n\
</specVersion>\r\n\
<device>\r\n\
<dlna:X_DLNADOC>DMR-1.50</dlna:X_DLNADOC>\r\n\
<deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>\r\n\
<presentationURL>/</presentationURL>\r\n\
<friendlyName>%s</friendlyName>\r\n\
<manufacturer>YUXING CO,Ltd</manufacturer>\r\n\
<manufacturerURL>http://www.yuxing.com.cn/labs</manufacturerURL>\r\n\
<modelDescription>UPnP/AV Media Renderer Device</modelDescription>\r\n\
<modelName>UPnP Renderer</modelName>\r\n\
<modelURL>http://www.yuxing.com.cn/labs</modelURL>\r\n\
<UDN>uuid:2e1e9f8e-97ec-44f7-ab3f-%s</UDN>\r\n\
<serviceList>\r\n\
<service>\r\n\
<serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>\r\n\
<serviceId>urn:upnp-org:serviceId:AVTransport</serviceId>\r\n\
<SCPDURL>/AVTransport_scpd.xml</SCPDURL>\r\n\
<controlURL>_urn:schemas-upnp-org:service:AVTransport_control</controlURL>\r\n\
<eventSubURL>/2e1e9f8e-97ec-44f7-ab3f-%s/AVTransport_Subscribe/</eventSubURL>\r\n\
</service>\r\n\
<service>\r\n\
<serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>\r\n\
<serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>\r\n\
<SCPDURL>/ConnectionManager_scpd.xml</SCPDURL>\r\n\
<controlURL>_urn:schemas-upnp-org:service:ConnectionManager_control</controlURL>\r\n\
<eventSubURL>/2e1e9f8e-97ec-44f7-ab3f-%s/ConnectionManager_Subscribe/</eventSubURL>\r\n\
</service>\r\n\
<service>\r\n\
<serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>\r\n\
<serviceId>urn:upnp-org:serviceId:RenderingControl</serviceId>\r\n\
<SCPDURL>/RenderingControl_scpd.xml</SCPDURL>\r\n\
<controlURL>_urn:schemas-upnp-org:service:RenderingControl_control</controlURL>\r\n\
<eventSubURL>/2e1e9f8e-97ec-44f7-ab3f-%s/RenderingControl_Subscribe/</eventSubURL>\r\n\
</service>\r\n\
<service>\r\n\
<serviceType>urn:schemas-upnp-org:service:X-CTC_Subscribe:1</serviceType>\r\n\
<serviceId>urn:upnp-org:serviceId:X-CTC_Subscribe</serviceId>\r\n\
<SCPDURL>/X-CTC_Subscribe_scpd.xml</SCPDURL>\r\n\
<controlURL>_urn:schemas-upnp-org:service:X-CTC_Subscribe_control</controlURL>\r\n\
<eventSubURL>/2e1e9f8e-97ec-44f7-ab3f-%s/X-CTC_Subscribe_Subscribe/</eventSubURL>\r\n\
</service>\r\n\
</serviceList>\r\n\
</device>\r\n\
</root>\r\n";
#endif

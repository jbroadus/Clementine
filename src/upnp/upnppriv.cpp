/* This file is part of Clementine.
   Copyright 2017, Jim Broadus <jbroadus@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "upnppriv.h"

#include <QFile>
#include <QDir>
#include <upnp/upnpdebug.h>

#include "upnpdesc.h"
#include "upnpmanager.h"
#include "core/logging.h"
#include "core/utilities.h"

#define MEDIA_RENDERER_DIR "MediaRenderer"
#define MEDIA_RENDERER_URN "urn:schemas-upnp-org:device:MediaRenderer:1"
#define MEDIA_RENDERER_NAME "Clementine"


UpnpManagerPriv::UpnpManagerPriv() :
  QObject(nullptr),
  clientHandle (-1),
  rendererHandle (-1),
  mgr_ (nullptr)
{
  int rc;

  //UpnpSetLogLevel(UPNP_ALL);
  rc = UpnpInit(NULL, 0);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpInit failed with " << rc;
    return;
  }

  webdir_ = QDir::toNativeSeparators(Utilities::GetConfigPath(Utilities::Path_Root) + "/upnp");
  if (!QFile::exists(webdir_)) {
    QDir dir;
    if (!dir.mkpath(webdir_)) {
      qLog(Error) << "Could not create " << webdir_;
      return;
    }
  }

  
  qLog(Debug) << "Web root is" << webdir_;
  rc = UpnpSetWebServerRootDir((const char *)webdir_.toAscii().data());
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpSetWebSererRootDir failed with " << rc;
    return;
  }
}

bool UpnpManagerPriv::BuildRendererInfo(UpnpDeviceInfo &info)
{
  UpnpServiceInfo service;

  renderer_info_.type = MEDIA_RENDERER_URN;
  renderer_info_.name = MEDIA_RENDERER_NAME;
  renderer_info_.udn = "uuid:clementine";

  service.type = "urn:schemas-upnp-org:service:RenderingControl:1";
  service.id = "urn:upnp-org:serviceId:RenderingControl";
  service.scpdUrl = "/MediaRenderer/RenderingControl.xml";
  renderer_info_.services << service;

  service.type = "urn:schemas-upnp-org:service:ConnectionManager:1";
  service.id = "urn:upnp-org:serviceId:ConnectionManager";
  service.scpdUrl = "/MediaRenderer/ConnectionManager.xml";
  renderer_info_.services << service;

  service.type = "urn:schemas-upnp-org:service:AVTransport:1";
  service.id = "urn:upnp-org:serviceId:AVTransport";
  service.scpdUrl = "/MediaRenderer/AVTransport.xml";
  renderer_info_.services << service;

  return true;
}

void UpnpManagerPriv::SetMgr(UpnpManager *mgr)
{
  mgr_ = mgr;

  CreateClient();
  CreateRenderer();
}

bool UpnpManagerPriv::CreateClient()
{
  int rc;
  rc = UpnpRegisterClient(ClientEventCallback, this, &clientHandle);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to register client :" << rc;
    return false;
  }

  qLog(Debug) << "Starting async search";
  rc = UpnpSearchAsync(clientHandle, 30, "ssdp:all", this);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to start asnc search: " << rc;
  }
  //UpnpSearchAsync(myHandle, 30, "urn:schemas-upnp-org:device:MediaServer:1", this);

  return true;
}

bool UpnpManagerPriv::CreateRenderer()
{
  int rc;
  char *addr = NULL;
  unsigned short port = 0;

  BuildRendererInfo(renderer_info_);
  QString dir(MEDIA_RENDERER_DIR);
  UpnpDesc desc(webdir_, dir, renderer_info_);

  for (int i=0; i<renderer_info_.services.count(); i++) {
    UpnpDesc desc(webdir_, renderer_info_.services[i]);
  }

  addr = UpnpGetServerIpAddress();
  port = UpnpGetServerPort();

  QString url = QString("http://%1:%2/").arg(addr, QString::number(port)) +
    desc.GetUrlPath();
  qLog(Debug) << "My URL: "<< url;

  rc = UpnpRegisterRootDevice((const char *)url.toAscii().data(),
                              DeviceEventCallback, this, &rendererHandle);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to register root device: " << rc;
    return false;
  }

  rc = UpnpSendAdvertisement(rendererHandle, 100);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Advertise failed: " << rc;
    return false;
  }
  
  return true;
}

int UpnpManagerPriv::GetNodeStr(QString &str, IXML_Document *doc, const char *name)
{
  IXML_NodeList *nl;
  IXML_Node *n, *tn;
  nl = ixmlDocument_getElementsByTagName(doc, name);
  if (nl) {
    n = ixmlNodeList_item(nl, 0);
    tn = ixmlNode_getFirstChild(n);
    str = ixmlNode_getNodeValue(tn);
    ixmlNodeList_free(nl);
    return 0;
  }
  else {
    qLog(Error) << "Could not find tag " << name;
    return -1;
  }
}

int UpnpManagerPriv::GetNodeStr(QString &str, IXML_Element *el, const char *name)
{
  IXML_NodeList *nl;
  IXML_Node *n, *tn;
  nl = ixmlElement_getElementsByTagName(el, name);
  if (nl) {
    n = ixmlNodeList_item(nl, 0);
    tn = ixmlNode_getFirstChild(n);
    str = ixmlNode_getNodeValue(tn);
    ixmlNodeList_free(nl);
    return 0;
  }
  else {
    qLog(Error) << "Could not find tag " << name;
    return -1;
  }
}

void UpnpManagerPriv::GetServices(IXML_Document *doc, UpnpDeviceInfo *devInfo)
{
  IXML_NodeList *nl, *sl;
  nl = ixmlDocument_getElementsByTagName(doc, "serviceList");
  if (nl) {
    IXML_Node *tn = ixmlNodeList_item(nl, 0);
    sl = ixmlElement_getElementsByTagName((IXML_Element *)tn, "service");
    if (sl) {
      int n = ixmlNodeList_length(sl);
      for (int i=0; i<n; i++) {
        UpnpServiceInfo svcInfo;
        IXML_Element *s = (IXML_Element *)ixmlNodeList_item(sl, i);
        GetNodeStr(svcInfo.type, s, "serviceType");
        GetNodeStr(svcInfo.id, s, "serviceId");
        devInfo->services << svcInfo;
      }
      ixmlNodeList_free(sl);
    }
    ixmlNodeList_free(nl);
  }
}

void UpnpManagerPriv::ParseDoc(IXML_Document *doc)
{
  QString udn;
  if (GetNodeStr(udn, doc, "UDN") != 0) {
    return;
  }

  if (mgr_->FindDeviceByUdn(udn) == -1) {
    UpnpDeviceInfo info;
    info.udn = udn;
    GetNodeStr(info.name, doc, "friendlyName");
    GetNodeStr(info.type, doc, "deviceType");
    GetServices(doc, &info);
    emit AddDevice(info);
  }
  else {
    //qLog(Debug) << "Device already in list";
  }
}

int UpnpManagerPriv::DiscoveryCallback(Upnp_EventType EventType,
                                       struct Upnp_Discovery *discovery)
{
  IXML_Document *DescDoc = NULL;
  if (discovery->ErrCode != UPNP_E_SUCCESS) {
    qLog(Error) << "UPnP discovery error: " << discovery->ErrCode;
    return 0;
  }

  qLog(Debug) << discovery->Location;
  UpnpDownloadXmlDoc(discovery->Location, &DescDoc);
  ParseDoc(DescDoc);

  return 0;
}

int UpnpManagerPriv::ClientEventCallback(Upnp_EventType EventType, void *Event, void *Cookie)
{
  UpnpManagerPriv *priv = (UpnpManagerPriv *)Cookie;
  Q_ASSERT(priv);

  switch (EventType) {
  case UPNP_DISCOVERY_SEARCH_RESULT:
    qLog(Debug) << "UPNP_DISCOVERY_SEARCH_RESULT";
    return priv->DiscoveryCallback(EventType, (struct Upnp_Discovery *)Event);

  case UPNP_DISCOVERY_SEARCH_TIMEOUT:
    qLog(Debug) << "UPNP_DISCOVERY_SEARCH_TIMEOUT";
    break;

  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
#if 0
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE";
    return discoveryCallback(EventType, (struct Upnp_Discovery *)Event);
#endif
    break;
  default:
    qLog(Debug) << "ClientEventCallback event " << EventType;
    break;
  }
  return 0;
}

int UpnpManagerPriv::DeviceEventCallback(Upnp_EventType EventType, void *Event, void *Cookie)
{
  UpnpManagerPriv *priv = (UpnpManagerPriv *)Cookie;
  Q_ASSERT(priv);

  switch (EventType) {
  default:
    qLog(Debug) << "DeviceEventCallback event " << EventType;
    break;
  }
  return 0;
}

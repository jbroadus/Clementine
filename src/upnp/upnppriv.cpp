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

#ifdef ENABLE_UPNP_DEBUG
#include <upnp/upnpdebug.h>
#endif

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

#ifdef ENABLE_UPNP_DEBUG
  UpnpSetLogLevel(UPNP_ALL);
#endif  
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

UpnpActionArgInfo *UpnpManagerPriv::AddActionArg(UpnpActionInfo *info, const char *name, const char *related, UpnpActionArgInfo::direction_t dir)
{
  UpnpActionArgInfo arg;
  arg.name = name;
  arg.relStateVar = related;
  arg.direction = dir;
  info->args << arg;
  return &info->args.last();
}

UpnpActionInfo *UpnpManagerPriv::AddAction(UpnpServiceInfo *info, const char *name)
{
  UpnpActionInfo action;
  action.name = name;
  info->actions << action;
  return &info->actions.last();
}

UpnpStateVarInfo *UpnpManagerPriv::AddStateVar(UpnpServiceInfo *info, const char *name, bool sendEvents,
                                               UpnpStateVarInfo::datatype_t type)
{
  UpnpStateVarInfo stateVar;
  stateVar.name = name;
  stateVar.dataType = type;
  info->stateVars << stateVar;
  return &info->stateVars.last();
}

UpnpServiceInfo *UpnpManagerPriv::AddService(UpnpDeviceInfo &info, const char *name)
{
  UpnpServiceInfo service;
  service.type = QString("urn:schemas-upnp-org:service:%1:1").arg(name);
  service.id = QString("urn:upnp-org:serviceId:%1").arg(name);
  service.scpdUrl = QString("/MediaRenderer/%1.xml").arg(name);
  service.eventSubUrl = QString("/MediaRenderer/%1/eventsub").arg(name);
  service.controlUrl = QString("/MediaRenderer/%1/control").arg(name);
  info.services << service;
  return &info.services.last();
}

bool UpnpManagerPriv::AddRenderingControlService(UpnpDeviceInfo &info)
{
  //UpnpServiceInfo *service;
  AddService(renderer_info_, "RenderingControl");
  return true;
}

bool UpnpManagerPriv::AddConnectionManagerService(UpnpDeviceInfo &info)
{
  //UpnpServiceInfo *service;
  AddService(renderer_info_, "ConnectionManager");
  return true;
}

bool UpnpManagerPriv::AddAvTransportService(UpnpDeviceInfo &info)
{
  UpnpServiceInfo *service;
  UpnpActionInfo *action;
  //UpnpStateVarInfo *stateVar;

  service = AddService(renderer_info_, "AVTransport");

  action = AddAction(service, "SetAVTransportURI");
  AddActionArg(action, "InstanceID", "A_ARG_TYPE_InstanceID", UpnpActionArgInfo::DIR_IN);
  AddActionArg(action, "CurrentURI", "AVTransportURI", UpnpActionArgInfo::DIR_IN);
  AddActionArg(action, "CurrentURIMetaData", "AVTransportURIMetaData", UpnpActionArgInfo::DIR_IN);

  action = AddAction(service, "SetNextAVTransportURI"); /* Optional */
  AddActionArg(action, "InstanceID", "A_ARG_TYPE_InstanceID", UpnpActionArgInfo::DIR_IN);
  AddActionArg(action, "NexURI", "NextAVTransportURI", UpnpActionArgInfo::DIR_IN);
  AddActionArg(action, "NextURIMetaData", "NextAVTransportURIMetaData", UpnpActionArgInfo::DIR_IN);

  action = AddAction(service, "GetMediaInfo");
  AddActionArg(action, "InstanceID", "A_ARG_TYPE_InstanceID", UpnpActionArgInfo::DIR_IN);

  action = AddAction(service, "GetTransportInfo");
  AddActionArg(action, "InstanceID", "A_ARG_TYPE_InstanceID", UpnpActionArgInfo::DIR_IN);
  AddActionArg(action, "CurrentTransportState", "TransportState", UpnpActionArgInfo::DIR_OUT);
  AddActionArg(action, "CurrentTransportStatus", "TransportStatus", UpnpActionArgInfo::DIR_OUT);
  AddActionArg(action, "CurrentSpeed", "TransportPlaySpeed", UpnpActionArgInfo::DIR_OUT);

  action = AddAction(service, "GetPositionInfo");
  action = AddAction(service, "GetDeviceCapabilities");
  action = AddAction(service, "GetTransportSettings");
  action = AddAction(service, "Stop");
  AddActionArg(action, "InstanceID", "A_ARG_TYPE_InstanceID", UpnpActionArgInfo::DIR_IN);
  action = AddAction(service, "Play");
  action = AddAction(service, "Pause"); /* Optional */
  /* Record optional */
  action = AddAction(service, "Seek");
  action = AddAction(service, "Next");
  action = AddAction(service, "Previous");
  /* SetPlayMode optional */
  /* SetRecordQualityMode optional */
  /* GetCurrentTransportActions optional */

  AddStateVar(service, "A_ARG_TYPE_InstanceID", false, UpnpStateVarInfo::TYPE_UI4);
  AddStateVar(service, "AVTransportURI", false, UpnpStateVarInfo::TYPE_STR);
  AddStateVar(service, "AVTransportURIMetaData", false, UpnpStateVarInfo::TYPE_STR);
  AddStateVar(service, "NextAVTransportURI", false, UpnpStateVarInfo::TYPE_STR);
  AddStateVar(service, "NextAVTransportURIMetaData", false, UpnpStateVarInfo::TYPE_STR);
  AddStateVar(service, "TransportState", false, UpnpStateVarInfo::TYPE_STR);
  AddStateVar(service, "TransportStatus", false, UpnpStateVarInfo::TYPE_STR);
  AddStateVar(service, "TransportPlaySpeed", false, UpnpStateVarInfo::TYPE_STR);
  return true;
}

bool UpnpManagerPriv::BuildRendererInfo(UpnpDeviceInfo &info)
{
  renderer_info_.type = MEDIA_RENDERER_URN;
  renderer_info_.name = MEDIA_RENDERER_NAME;
  renderer_info_.udn = "uuid:clementine";

  AddRenderingControlService(renderer_info_);
  AddConnectionManagerService(renderer_info_);
  AddAvTransportService(renderer_info_);

  return true;
}

void UpnpManagerPriv::SetMgr(UpnpManager *mgr)
{
  mgr_ = mgr;

  CreateClient();
  CreateRenderer();
}

#define SEARCH_PATTERN "ssdp:all"
//#define SEARCH_PATTERN "urn:schemas-upnp-org:device:MediaServer:1"
bool UpnpManagerPriv::StartAsyncSearch()
{
  int rc;
  qLog(Debug) << "Starting async search";
  rc = UpnpSearchAsync(clientHandle, 30, SEARCH_PATTERN, this);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to start asnc search: " << rc;
    return false;
  }

  return true;
}

bool UpnpManagerPriv::CreateClient()
{
  int rc;
  rc = UpnpRegisterClient(ClientEventCallback, this, &clientHandle);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to register client :" << rc;
    return false;
  }

  StartAsyncSearch();

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
                              RendererEventCallback, this, &rendererHandle);
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
                                       UpnpDiscovery *discovery)
{
  IXML_Document *DescDoc = NULL;
  int err = UpnpDiscovery_get_ErrCode(discovery);
  if (err != UPNP_E_SUCCESS) {
    qLog(Error) << "UPnP discovery error: " << err;
    return 0;
  }

  const char *loc = UpnpString_get_String(UpnpDiscovery_get_Location(discovery));
  //qLog(Debug) << loc;
  UpnpDownloadXmlDoc(loc, &DescDoc);
  ParseDoc(DescDoc);

  return 0;
}

int UpnpManagerPriv::ActionReqCallback(UpnpActionRequest *request)
{
  QString sid(UpnpString_get_String(UpnpActionRequest_get_ServiceID(request)));
  QString aname(UpnpString_get_String(UpnpActionRequest_get_ActionName(request)));

  qLog(Debug) << "Action " << aname << " on " << sid;
  UpnpServiceInfo *service = renderer_info_.FindServiceById(sid);
  if (!service) {
    qLog(Error) << "Could not find service " << sid;
    return -1;
  }
  UpnpActionInfo *action = service->FindActionByName(aname);
  if (!action) {
    qLog(Error) << "Could not find action " << aname;
    return -1;
  }

  IXML_Node *req = (IXML_Node *)UpnpActionRequest_get_ActionRequest(request);
  char *xmlbuff = ixmlPrintNode(req);
  if (xmlbuff) {
    qLog(Debug) << xmlbuff;
    ixmlFreeDOMString(xmlbuff);
  }
  xmlbuff = ixmlPrintNode(req);
  if (xmlbuff) {
    qLog(Debug) << xmlbuff;
    ixmlFreeDOMString(xmlbuff);
  }
  return 0;
}

int UpnpManagerPriv::ClientEventCallback(Upnp_EventType EventType, const void *Event, void *Cookie)
{
  UpnpManagerPriv *priv = (UpnpManagerPriv *)Cookie;
  Q_ASSERT(priv);

  switch (EventType) {
  case UPNP_DISCOVERY_SEARCH_RESULT:
    //qLog(Debug) << "UPNP_DISCOVERY_SEARCH_RESULT";
    return priv->DiscoveryCallback(EventType, (UpnpDiscovery *)Event);

  case UPNP_DISCOVERY_SEARCH_TIMEOUT:
    qLog(Debug) << "Search timeout. Restarting";
    priv->StartAsyncSearch();
    break;

  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
#if 0
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE";
    return discoveryCallback(EventType, (UpnpDiscovery *)Event);
#endif
    break;
  case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
#if 0
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE";
#endif
    break;
  default:
    qLog(Debug) << "ClientEventCallback event " << EventType;
    break;
  }
  return 0;
}

int UpnpManagerPriv::RendererEventCallback(Upnp_EventType EventType, const void *Event, void *Cookie)
{
  UpnpManagerPriv *priv = (UpnpManagerPriv *)Cookie;
  Q_ASSERT(priv);

  switch (EventType) {
  case UPNP_CONTROL_ACTION_REQUEST:
    return priv->ActionReqCallback((UpnpActionRequest *)Event);
    break;
  default:
    qLog(Debug) << "DeviceEventCallback event " << EventType;
    break;
  }
  return 0;
}

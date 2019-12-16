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

#include <upnp/upnptools.h>
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

class UpnpDescDoc;

class UpnpDescBase
{
public:
  UpnpDescBase(UpnpDescDoc *top) :
    top_(top) {}

  bool GetNodeStr(QString &str, const char *tag)
  {
    IXML_Node *n, *tn;
    IXML_NodeList *nl = GetElementList(tag);
    if (nl) {
      n = ixmlNodeList_item(nl, 0);
      tn = ixmlNode_getFirstChild(n);
      str = ixmlNode_getNodeValue(tn);
      ixmlNodeList_free(nl);
      //qLog(Debug) << "Found " << tag << " is " << str;
      return true;
    }
    else {
      //qLog(Error) << "Could not find tag " << tag;
      return false;
    }
  }

  bool GetAbsUrl(QString &str, const char *name);

  virtual IXML_NodeList *GetElementList(const char *tag) = 0;

  UpnpDescDoc *top_;
};

class UpnpDescDoc : public UpnpDescBase
{
 public:
  UpnpDescDoc(IXML_Document *doc = NULL) :
    UpnpDescBase(this),
    doc_(doc),
    free_doc_(false)
  {
  }

  ~UpnpDescDoc()
  {
    if (free_doc_)
      ixmlDocument_free(doc_);
  }

  virtual IXML_NodeList *GetElementList(const char *tag)
  {
    return ixmlDocument_getElementsByTagName(doc_, tag);
  }

  bool Download(const char *url)
  {
    if (free_doc_) {
      ixmlDocument_free(doc_);
      free_doc_ = false;
    }

    url_.setUrl(url);

    UpnpDownloadXmlDoc(url, &doc_);

    if (doc_ != NULL) {
      free_doc_ = true;
      return true;
    }
    else {
      return false;
    }
  }

  bool Download(QString url)
  {
    return Download(url.toLatin1().data());
  }

  void Print()
  {
    if (!doc_)
      return;

    char *xmlbuff = ixmlPrintNode((IXML_Node *)doc_);
    if (xmlbuff) {
      qLog(Debug) << xmlbuff;
      ixmlFreeDOMString(xmlbuff);
    }
  }

  QUrl url_;
  IXML_Document *doc_;
  bool free_doc_;
};

class UpnpDescElement : public UpnpDescBase
{
public:
  UpnpDescElement(UpnpDescDoc *top, IXML_Element *elem) :
    UpnpDescBase(top),
    elem_(elem) {}

  virtual IXML_NodeList *GetElementList(const char *tag)
  {
    return ixmlElement_getElementsByTagName(elem_, tag);
  }

  IXML_Element *elem_;
};

bool UpnpDescBase::GetAbsUrl(QString &str, const char *name)
{
  if (!GetNodeStr(str, name))
    return false;

  QUrl res = top_->url_.resolved(QUrl(str));
  str = res.toString();

  return true;
}




class UpnpElementList
{
public:
  UpnpElementList(UpnpDescDoc &doc, const char *listTag, const char *itemTag) :
    nodeList(nullptr),
    nodeCount(0)
  {
    IXML_NodeList *listList =
      ixmlDocument_getElementsByTagName(doc.doc_, listTag);
    if (listList) {
      Init(listList, itemTag);
      ixmlNodeList_free(listList);
    }
  }

  UpnpElementList(UpnpDescElement &elem, const char *listTag, const char *itemTag) :
    nodeList(nullptr),
    nodeCount(0)
  {
    IXML_NodeList *listList =
      ixmlElement_getElementsByTagName(elem.elem_, listTag);
    if (listList) {
      Init(listList, itemTag);
      ixmlNodeList_free(listList);
    }
  }

  ~UpnpElementList()
  {
    if (nodeList)
      ixmlNodeList_free(nodeList);
  }

  unsigned int count()
  {
    return nodeCount;
  }

  IXML_Element *get(unsigned int index)
  {
    if (!nodeList || index >= nodeCount)
      return nullptr;
    return (IXML_Element *)ixmlNodeList_item(nodeList, index);
  }

private:
  /* Common constructor bits. */
  void Init(IXML_NodeList *listList, const char *itemTag)
  {
    IXML_Node *tn = ixmlNodeList_item(listList, 0);
    nodeList = ixmlElement_getElementsByTagName((IXML_Element *)tn, itemTag);
    if(nodeList)
      nodeCount = ixmlNodeList_length(nodeList);
  }

  IXML_NodeList *nodeList;
  unsigned int nodeCount;
};


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
  rc = UpnpSetWebServerRootDir((const char *)webdir_.toLatin1().data());
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpSetWebSererRootDir failed with " << rc;
    return;
  }
}

UpnpActionArgInfo *UpnpManagerPriv::AddActionArg(UpnpActionInfo *action,
                                                 const char *name,
                                                 UpnpStateVarInfo *related,
                                                 bool input)
{
  UpnpActionArgInfo arg;
  arg.name = name;
  arg.relStateVar = related;
  if (input) {
    action->in_args << arg;
    return &action->in_args.last();
  }
  else {
    action->out_args << arg;
    return &action->out_args.last();
  }
}

UpnpActionArgInfo *UpnpManagerPriv::AddActionArg(UpnpActionInfo *action,
                                                 UpnpServiceInfo *service,
                                                 UpnpDescElement &element)
{
  QString nameStr;
  QString relatedVar;
  QString dirStr;
  element.GetNodeStr(nameStr, "name");
  element.GetNodeStr(relatedVar, "relatedStateVariable");
  element.GetNodeStr(dirStr, "direction");
  bool dir = (dirStr.compare("in", Qt::CaseInsensitive) == 0);
  UpnpStateVarInfo *stateVar = service->FindStateVarByName(relatedVar);
  return AddActionArg(action, nameStr.toLatin1().data(), stateVar, dir);
}

UpnpActionInfo *UpnpManagerPriv::AddAction(UpnpServiceInfo *info,
                                           const char *name,
                                           UpnpActionInfo::id_t id)
{
  UpnpActionInfo action;
  action.name = name;
  action.id = id;
  info->actions << action;
  return &info->actions.last();
}

UpnpActionInfo *UpnpManagerPriv::AddAction(UpnpServiceInfo *service,
                                           UpnpDescElement &elem)
{
  QString name;
  elem.GetNodeStr(name, "name");
  //qLog(Debug) << "Action " << name;
  UpnpActionInfo *action = AddAction(service, name.toLatin1().data(), UpnpActionInfo::ID_Unknown /*todo*/);
  return action;
}

UpnpStateVarInfo *UpnpManagerPriv::AddStateVar(UpnpServiceInfo *service,
                                               const char *name,
                                               bool sendEvents,
                                               UpnpStateVarInfo::datatype_t type)
{
  UpnpStateVarInfo stateVar;
  stateVar.name = name;
  stateVar.dataType = type;
  service->stateVars << stateVar;
  return &service->stateVars.last();
}

UpnpStateVarInfo *UpnpManagerPriv::AddStateVar(UpnpServiceInfo *service,
                                               UpnpDescElement &elem)
{
  QString nameStr, dataTypeStr, sendEventAttrStr;
  elem.GetNodeStr(nameStr, "name");
  elem.GetNodeStr(sendEventAttrStr, "sendEventsAttribute");
  elem.GetNodeStr(dataTypeStr, "dataType");

  bool sendEventAttr = (sendEventAttrStr.compare("yes",
                                                 Qt::CaseInsensitive) == 0);
  UpnpStateVarInfo::datatype_t dataType;
  if (dataTypeStr.compare("ui4", Qt::CaseInsensitive) == 0)
    dataType = UpnpStateVarInfo::TYPE_UI4;
  else if (dataTypeStr.compare("i4", Qt::CaseInsensitive) == 0)
    dataType = UpnpStateVarInfo::TYPE_I4;
  else
    dataType = UpnpStateVarInfo::TYPE_STR;

  return AddStateVar(service, nameStr.toLatin1().data(),
                     sendEventAttr, dataType);
}

UpnpServiceInfo *UpnpManagerPriv::AddService(UpnpDeviceInfo &info,
                                             UpnpServiceInfo &service)
{
  info.services << service;

  if (service.type == "urn:schemas-upnp-org:service:ContentDirectory:1")
    info.flags =
      UpnpDeviceInfo::flags_t(info.flags | UpnpDeviceInfo::FLAG_SERVER);
  if (service.type == "urn:schemas-upnp-org:service:AVTransport:1")
    info.flags =
      UpnpDeviceInfo::flags_t(info.flags | UpnpDeviceInfo::FLAG_PLAYER);

  return &info.services.last();
}

UpnpServiceInfo *UpnpManagerPriv::AddService(UpnpDeviceInfo &info,
                                             UpnpDescElement &element)
{
  UpnpServiceInfo service;

  element.GetNodeStr(service.type, UpnpDesc::tag_serviceType);
  element.GetNodeStr(service.id, UpnpDesc::tag_serviceId);
  element.GetAbsUrl(service.scpdUrl, UpnpDesc::tag_SCPDURL);
  element.GetAbsUrl(service.eventSubUrl, UpnpDesc::tag_eventSubURL);
  element.GetAbsUrl(service.controlUrl, UpnpDesc::tag_controlURL);

  return AddService(info, service);
}

UpnpServiceInfo *UpnpManagerPriv::AddService(UpnpDeviceInfo &info,
                                             const char *name)
{
  UpnpServiceInfo service;
  service.type = QString("urn:schemas-upnp-org:service:%1:1").arg(name);
  service.id = QString("urn:upnp-org:serviceId:%1").arg(name);
  service.scpdUrl = QString("/MediaRenderer/%1.xml").arg(name);
  service.eventSubUrl = QString("/MediaRenderer/%1/eventsub").arg(name);
  service.controlUrl = QString("/MediaRenderer/%1/control").arg(name);

  return AddService(info, service);
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

#define ADDVAR(name, send, type) \
  UpnpStateVarInfo *name = AddStateVar(service, #name, send, type)
#define ADDACT(name) \
  AddAction(service, #name, UpnpActionInfo::ID_##name)
bool UpnpManagerPriv::AddAvTransportService(UpnpDeviceInfo &info)
{
  UpnpServiceInfo *service;
  UpnpActionInfo *action;

  service = AddService(info, "AVTransport");
  ADDVAR(A_ARG_TYPE_InstanceID, false, UpnpStateVarInfo::TYPE_UI4);
  ADDVAR(AbsoluteCounterPosition, false, UpnpStateVarInfo::TYPE_I4);
  ADDVAR(AbsoluteTimePosition, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(AVTransportURI, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(AVTransportURIMetaData, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(CurrentTrack, false, UpnpStateVarInfo::TYPE_UI4);
  ADDVAR(CurrentTrackDuration, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(CurrentTrackMetaData, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(CurrentTrackURI, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(NextAVTransportURI, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(NextAVTransportURIMetaData, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(RelativeCounterPosition, false, UpnpStateVarInfo::TYPE_I4);
  ADDVAR(RelativeTimePosition, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(TransportState, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(TransportStatus, false, UpnpStateVarInfo::TYPE_STR);
  ADDVAR(TransportPlaySpeed, false, UpnpStateVarInfo::TYPE_STR);

  action = ADDACT(SetAVTransportURI);
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);
  AddActionArg(action, "CurrentURI", AVTransportURI, true);
  AddActionArg(action, "CurrentURIMetaData", AVTransportURIMetaData, true);

  action = ADDACT(SetNextAVTransportURI); /* Optional */
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);
  AddActionArg(action, "NexURI", NextAVTransportURI, true);
  AddActionArg(action, "NextURIMetaData", NextAVTransportURIMetaData, true);

  action = ADDACT(GetMediaInfo);
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);

  action = ADDACT(GetTransportInfo);
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);
  AddActionArg(action, "CurrentTransportState", TransportState, false);
  AddActionArg(action, "CurrentTransportStatus", TransportStatus, false);
  AddActionArg(action, "CurrentSpeed", TransportPlaySpeed, false);

  action = ADDACT(GetPositionInfo);
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);
  AddActionArg(action, "Track", CurrentTrack, false);
  AddActionArg(action, "TrackDuration", CurrentTrackDuration, false);
  AddActionArg(action, "TrackMetaData", CurrentTrackMetaData, false);
  AddActionArg(action, "TrackURI", CurrentTrackURI, false);
  AddActionArg(action, "RelTime", RelativeTimePosition, false);
  AddActionArg(action, "AbsTime", AbsoluteTimePosition, false);
  AddActionArg(action, "RelCount", RelativeCounterPosition, false);
  AddActionArg(action, "AbsCount", AbsoluteCounterPosition, false);

  action = ADDACT(GetDeviceCapabilities);

  action = ADDACT(GetTransportSettings);

  action = ADDACT(Stop);
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);

  action = ADDACT(Play);
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);
  AddActionArg(action, "Speed", TransportPlaySpeed, true);

  action = ADDACT(Pause); /* Optional */
  AddActionArg(action, "InstanceID", A_ARG_TYPE_InstanceID, true);

  /* Record optional */

  action = ADDACT(Seek);

  action = ADDACT(Next);

  action = ADDACT(Previous);

  /* SetPlayMode optional */
  /* SetRecordQualityMode optional */
  /* GetCurrentTransportActions optional */

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

  /* Generate desc */
  UpnpDesc desc(webdir_, dir, renderer_info_);

  UpnpServiceList::iterator svc_itr;
  for (svc_itr = renderer_info_.services.begin();
       svc_itr != renderer_info_.services.end();
       svc_itr++) {
    /* Generate scpd */
    UpnpDesc scpd(webdir_, *svc_itr);
  }

  addr = UpnpGetServerIpAddress();
  port = UpnpGetServerPort();

  QString url = QString("http://%1:%2/").arg(addr, QString::number(port)) +
    desc.GetUrlPath();
  qLog(Debug) << "My URL: "<< url;

  rc = UpnpRegisterRootDevice((const char *)url.toLatin1().data(),
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

void UpnpManagerPriv::DownloadSpcd(UpnpServiceInfo *service)
{
  UpnpDescDoc doc;
  if (doc.Download(service->scpdUrl)) {
    ParseSpcd(doc, service);
  }
  else {
    qLog(Error) << "Failed to download " << service->scpdUrl;
  }
}

void UpnpManagerPriv::GetServicesFromDesc(UpnpDescDoc &doc, UpnpDeviceInfo *devInfo)
{
  UpnpElementList list(doc, "serviceList", "service");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    UpnpServiceInfo *service = AddService(*devInfo, elem);
    DownloadSpcd(service);
  }
}

void UpnpManagerPriv::ParseDeviceDesc(UpnpDescDoc &doc)
{
  QString udn;
  if (!doc.GetNodeStr(udn, "UDN")) {
    return;
  }

  if (mgr_->FindDeviceByUdn(udn) == -1) {
    UpnpDeviceInfo *info = new UpnpDeviceInfo;
    info->flags = UpnpDeviceInfo::FLAG_NONE;
    info->udn = udn;
    doc.GetNodeStr(info->name, "friendlyName");
    doc.GetNodeStr(info->type, "deviceType");
    GetServicesFromDesc(doc, info);

    /* UpnpManager takes ownership of info. */
    emit AddDevice(info);
  }
  else {
    //qLog(Debug) << "Device already in list";
  }
}

void UpnpManagerPriv::ParseSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service)
{
  GetStateTableFromSpcd(doc, service);
  GetActionsFromSpcd(doc, service);
}

void UpnpManagerPriv::GetActionArgsFromSpcd(UpnpDescElement &elem,
                                            UpnpActionInfo *action,
                                            UpnpServiceInfo *service)
{
  UpnpElementList list(elem, "argumentList", "argument");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement argElem(elem.top_, list.get(i));
    AddActionArg(action, service, argElem);
  }
}

void UpnpManagerPriv::GetActionsFromSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service)
{
  UpnpElementList list(doc, "actionList", "action");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    UpnpActionInfo *action = AddAction(service, elem);
    GetActionArgsFromSpcd(elem, action, service);
  }
}

void UpnpManagerPriv::GetStateTableFromSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service)
{
  UpnpElementList list(doc, "serviceStateTable", "stateVariable");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    AddStateVar(service, elem);
  }
}

int UpnpManagerPriv::DiscoveryCallback(Upnp_EventType EventType,
                                       UpnpDiscovery *discovery)
{
  int err = UpnpDiscovery_get_ErrCode(discovery);
  if (err != UPNP_E_SUCCESS) {
    qLog(Error) << "UPnP discovery error: " << err;
    return 0;
  }

  const char *loc = UpnpString_get_String(UpnpDiscovery_get_Location(discovery));
  //qLog(Debug) << loc;
  UpnpDescDoc doc;
  if (doc.Download(loc)) {
    ParseDeviceDesc(doc);
  }
  else {
    qLog(Error) << "Failed to download " << loc;
  }

  return 0;
}

int UpnpManagerPriv::ActionReqCallback(UpnpActionRequest *request)
{
  UpnpActionArgList::iterator arg_itr;

  QString sid(UpnpString_get_String(UpnpActionRequest_get_ServiceID(request)));
  QString aname(UpnpString_get_String(UpnpActionRequest_get_ActionName(request)));

  //qLog(Debug) << "Action " << aname << " on " << sid;
  UpnpServiceInfo *service = renderer_info_.FindServiceById(sid);
  if (!service) {
    qLog(Error) << "Could not find service " << sid << " for action " << aname;
    return -1;
  }
  UpnpActionInfo *action = service->FindActionByName(aname);
  if (!action) {
    qLog(Error) << "Could not find action " << aname << " in service " << sid;
    return -1;
  }

  IXML_Document *req = UpnpActionRequest_get_ActionRequest(request);
  UpnpDescDoc doc(req);

  /* Fill input args */
  for (arg_itr = action->in_args.begin();
       arg_itr != action->in_args.end();
       arg_itr++) {
    if (arg_itr->relStateVar) {
      doc.GetNodeStr(arg_itr->relStateVar->value,
                 arg_itr->name.toLatin1().data());
    }
  }

#ifdef ENABLE_UPNP_DEBUG
  doc.Print();
#endif

  /* Blocking */
  emit DoAction(action);

  /* Add output args */
  const char *name = UpnpString_get_String(UpnpActionRequest_get_ActionName(request));
  for (arg_itr = action->out_args.begin();
       arg_itr != action->out_args.end();
       arg_itr++) {
    if (arg_itr->relStateVar) {
      UpnpAddToActionResponse(&req, name,
                              service->type.toLatin1().data(),
                              arg_itr->name.toLatin1().data(),
                              arg_itr->relStateVar->value.toLatin1().data());
    }
  }

#ifdef ENABLE_UPNP_DEBUG
  UpnpDescDoc(req).Print();
#endif
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
#ifdef ENABLE_UPNP_DEBUG
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE";
    return discoveryCallback(EventType, (UpnpDiscovery *)Event);
#endif
    break;
  case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
#ifdef ENABLE_UPNP_DEBUG
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

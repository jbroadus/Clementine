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
#include <upnp/upnptools.h>

#include "upnpclient.h"
#include "upnpdesc.h"
#include "upnpmanager.h"
#include "upnpwrappers.h"
#include "core/logging.h"
#include "core/utilities.h"

#define MEDIA_RENDERER_DIR "MediaRenderer"
#define MEDIA_RENDERER_URN "urn:schemas-upnp-org:device:MediaRenderer:1"
#define MEDIA_RENDERER_NAME "Clementine"



UpnpManagerPriv::UpnpManagerPriv() :
  QObject(nullptr),
  client_(new UpnpClient(this)),
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

UpnpManagerPriv::~UpnpManagerPriv()
{
  delete client_;
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
  return AddActionArg(action, nameStr.toAscii().data(), stateVar, dir);
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
  UpnpActionInfo *action = AddAction(service, name.toAscii().data(), UpnpActionInfo::ID_Unknown /*todo*/);
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

  return AddStateVar(service, nameStr.toAscii().data(),
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
  client_->SetMgr(mgr);

  CreateRenderer();
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

int UpnpManagerPriv::ActionReqCallback(struct Upnp_Action_Request *request)
{
  UpnpActionArgList::iterator arg_itr;

  QString sid(request->ServiceID);
  QString aname(request->ActionName);

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

  UpnpDescDoc doc(request->ActionRequest);

  /* Fill input args */
  for (arg_itr = action->in_args.begin();
       arg_itr != action->in_args.end();
       arg_itr++) {
    if (arg_itr->relStateVar) {
      doc.GetNodeStr(arg_itr->relStateVar->value,
                 arg_itr->name.toAscii().data());
    }
  }

#if 1
  doc.Print();
#endif

  /* Blocking */
  emit DoAction(action);

  /* Add output args */
  for (arg_itr = action->out_args.begin();
       arg_itr != action->out_args.end();
       arg_itr++) {
    if (arg_itr->relStateVar) {
      UpnpAddToActionResponse(&request->ActionResult, request->ActionName,
                              service->type.toAscii().data(),
                              arg_itr->name.toAscii().data(),
                              arg_itr->relStateVar->value.toAscii().data());
    }
  }

#if 1
  UpnpDescDoc(request->ActionResult).Print();
#endif
  return 0;
}

int UpnpManagerPriv::RendererEventCallback(Upnp_EventType EventType, void *Event, void *Cookie)
{
  UpnpManagerPriv *priv = (UpnpManagerPriv *)Cookie;
  Q_ASSERT(priv);

  switch (EventType) {
  case UPNP_CONTROL_ACTION_REQUEST:
    return priv->ActionReqCallback((Upnp_Action_Request *)Event);
    break;
  default:
    qLog(Debug) << "DeviceEventCallback event " << EventType;
    break;
  }
  return 0;
}

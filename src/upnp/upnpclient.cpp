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

#include "upnpclient.h"
#include "upnppriv.h"
#include "upnpwrappers.h"
#include "core/logging.h"

UpnpClient::UpnpClient(UpnpManagerPriv *priv) :
  mgr_(nullptr),
  priv_(priv)
{
}

void UpnpClient::SetMgr(UpnpManager *mgr)
{
  mgr_ = mgr;

  int rc;
  rc = UpnpRegisterClient(EventCallback, this, &handle);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to register client :" << rc;
    return;
  }

  StartAsyncSearch();
}

#define SEARCH_PATTERN "ssdp:all"
//#define SEARCH_PATTERN "urn:schemas-upnp-org:device:MediaServer:1"
bool UpnpClient::StartAsyncSearch()
{
  int rc;
  qLog(Debug) << "Starting async search";
  rc = UpnpSearchAsync(handle, 30, SEARCH_PATTERN, this);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to start asnc search: " << rc;
    return false;
  }

  return true;
}

int UpnpClient::DiscoveryCallback(Upnp_EventType EventType,
                                  struct Upnp_Discovery *discovery)
{
  if (discovery->ErrCode != UPNP_E_SUCCESS) {
    qLog(Error) << "UPnP discovery error: " << discovery->ErrCode;
    return 0;
  }

  //qLog(Debug) << discovery->Location;
  UpnpDescDoc doc;
  if (doc.Download(discovery->Location)) {
    ParseDeviceDesc(doc);
  }
  else {
    qLog(Error) << "Failed to download " << discovery->Location;
  }

  return 0;
}

int UpnpClient::EventCallback(Upnp_EventType EventType, void *Event,
                                    void *Cookie)
{
  UpnpClient *client = (UpnpClient *)Cookie;
  Q_ASSERT(client);

  switch (EventType) {
  case UPNP_DISCOVERY_SEARCH_RESULT:
    //qLog(Debug) << "UPNP_DISCOVERY_SEARCH_RESULT";
    return client->DiscoveryCallback(EventType,
                                     (struct Upnp_Discovery *)Event);

  case UPNP_DISCOVERY_SEARCH_TIMEOUT:
    qLog(Debug) << "Search timeout. Restarting";
    client->StartAsyncSearch();
    break;

  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
#if 0
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE";
    return client->DiscoveryCallback(EventType,
                                     (struct Upnp_Discovery *)Event);
#endif
    break;
  case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
#if 0
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE";
#endif
    break;
  default:
    qLog(Debug) << "EventCallback event " << EventType;
    break;
  }
  return 0;
}

void UpnpClient::ParseDeviceDesc(UpnpDescDoc &doc)
{
  QString udn;
  if (!doc.GetNodeStr(udn, "UDN")) {
    return;
  }

  if (mgr_->FindDeviceByUdn(udn) == -1) {
    UpnpDeviceInfo *info = new UpnpDeviceInfo;
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

void UpnpClient::DownloadSpcd(UpnpServiceInfo *service)
{
  UpnpDescDoc doc;
  if (doc.Download(service->scpdUrl)) {
    ParseSpcd(doc, service);
  }
  else {
    qLog(Error) << "Failed to download " << service->scpdUrl;
  }
}

void UpnpClient::GetServicesFromDesc(UpnpDescDoc &doc, UpnpDeviceInfo *devInfo)
{
  UpnpElementList list(doc, "serviceList", "service");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    UpnpServiceInfo *service = priv_->AddService(*devInfo, elem);
    DownloadSpcd(service);
  }
}

void UpnpClient::ParseSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service)
{
  GetStateTableFromSpcd(doc, service);
  GetActionsFromSpcd(doc, service);
}

void UpnpClient::GetActionArgsFromSpcd(UpnpDescElement &elem,
                                            UpnpActionInfo *action,
                                            UpnpServiceInfo *service)
{
  UpnpElementList list(elem, "argumentList", "argument");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement argElem(elem.top_, list.get(i));
    priv_->AddActionArg(action, service, argElem);
  }
}

void UpnpClient::GetActionsFromSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service)
{
  UpnpElementList list(doc, "actionList", "action");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    UpnpActionInfo *action = priv_->AddAction(service, elem);
    GetActionArgsFromSpcd(elem, action, service);
  }
}

void UpnpClient::GetStateTableFromSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service)
{
  UpnpElementList list(doc, "serviceStateTable", "stateVariable");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    priv_->AddStateVar(service, elem);
  }
}


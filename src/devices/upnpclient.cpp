/* This file is part of Clementine.
   Copyright 2019, Jim Broadus <jbroadus@gmail.com>

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
#include "upnpdoc.h"

#include <QDir>
#include <QFile>

#include <upnp/upnptools.h>
#ifdef ENABLE_UPNP_DEBUG
#include <upnp/upnpdebug.h>
#endif
#include "upnpdeviceinfo.h"
#include "upnpserviceinfo.h"

#include "core/logging.h"
#include "core/utilities.h"


bool UpnpDescBase::GetAbsUrl(QUrl &url, const char *name)
{
  QString url_str;
  if (!GetNodeStr(url_str, name))
    return false;

  url = top_->url_.resolved(QUrl(url_str));
  return true;
}



UpnpClient::UpnpClient() :
  QObject(nullptr),
  client_handle_(-1)
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
}

#define SEARCH_PATTERN "ssdp:all"
//#define SEARCH_PATTERN "urn:schemas-upnp-org:device:MediaServer:1"
bool UpnpClient::StartAsyncSearch()
{
  int rc;
  qLog(Debug) << "Starting async search";
  rc = UpnpSearchAsync(client_handle_, 30, SEARCH_PATTERN, this);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to start asnc search: " << rc;
    return false;
  }

  return true;
}

bool UpnpClient::SendAction(UpnpActionInfo *action, UpnpServiceInfo *service)
{
  QByteArray url = service->control_url_.toEncoded();
  QByteArray type = service->type_.toAscii();
  QByteArray name = action->name_.toAscii();
  IXML_Document *req_doc = NULL;
  IXML_Document *res_doc = NULL;
  if (action->in_args_.size() > 0) {
    for (UpnpActionArgInfo& arg : action->in_args_) {
      QByteArray arg_name = arg.name_.toAscii();
      QByteArray arg_val = arg.rel_state_var_->value_.toAscii();
      qLog(Debug) << "Arg " << arg.name_ << "=" << arg.rel_state_var_->value_;
      UpnpAddToAction (&req_doc, name.data(), type.data(),
                       arg_name.data(),arg_val.data());
    }
  } else {
    qLog(Debug) << "No args for " << name;
    UpnpMakeAction(name.data(), type.data(), 0, NULL);
  }
  int rc = UpnpSendAction(client_handle_, url.data(), type.data(),
                          NULL, /* must be NULL per upnp.h */
                          req_doc, &res_doc);
  if (rc == UPNP_E_SUCCESS) {
    UpnpDescDoc res(res_doc);
    //res.Print();
    action->UpdateFromResponse(res);
  }
  else {
    qLog(Error) << "UpnpSendAction failed :" << rc;
    //UpnpDescDoc(req_doc).Print();
  }
  ixmlDocument_free(req_doc);
  ixmlDocument_free(res_doc);
  return (rc == UPNP_E_SUCCESS);
}

bool UpnpClient::Start()
{
  int rc;
  rc = UpnpRegisterClient(ClientEventCallback, this, &client_handle_);
  if (rc != UPNP_E_SUCCESS) {
    qLog(Error) << "Failed to register client :" << rc;
    return false;
  }

  StartAsyncSearch();

  return true;
}

QStringList UpnpClient::GetDeviceIDs()
{
  QMutexLocker l(&mutex_);
  return devices_.keys();
}

std::shared_ptr<UpnpDeviceInfo> UpnpClient::GetDevice(const QString& id)
{
  QMutexLocker l(&mutex_);
  if (!devices_.contains(id)) {
    qLog(Error) << "No device " << id;
    return NULL;
  }
  return devices_[id];
}

std::shared_ptr<UpnpDeviceInfo> UpnpClient::GetDevice(const QUrl& url)
{
  QMutexLocker l(&mutex_);
  for (std::shared_ptr<UpnpDeviceInfo> info : devices_) {
    if (info->url_ == url)
      return info;
  }
  return NULL;
}

int UpnpClient::DiscoveryCallback(Upnp_EventType event_type,
                                       UpnpDiscovery *discovery)
{
  QMutexLocker l(&mutex_);
  int err = UpnpDiscovery_get_ErrCode(discovery);
  if (err != UPNP_E_SUCCESS) {
    qLog(Error) << "UPnP discovery error: " << err;
    return 0;
  }

  const char *id =
    UpnpString_get_String(UpnpDiscovery_get_DeviceID(discovery));
  if (devices_.contains(id)) {
    //qLog(Debug) << "Already have ID " << id;
    return 0;
  }

  const char *loc =
    UpnpString_get_String(UpnpDiscovery_get_Location(discovery));
  QUrl url(loc);
  UpnpDeviceInfo *info = MakeDeviceInfo(url);
  if (info) {
    devices_[id] = std::shared_ptr<UpnpDeviceInfo>(info);
    emit DeviceDiscovered(QString(id));
  }
  return 0;
}

UpnpDeviceInfo *UpnpClient::MakeDeviceInfo(const QUrl& location)
{
  UpnpDescDoc doc;
  if (doc.Download(location)) {
    UpnpDeviceInfo *info = new UpnpDeviceInfo;
    info->url_ = location;
    if (info->Populate(doc))
      return info;
    else
      delete info;
  }
  else {
    qLog(Error) << "Failed to download " << location;
  }
  return NULL;
}

int UpnpClient::ClientEventCallback(Upnp_EventType EventType, const void *Event, void *Cookie)
{
  UpnpClient *priv = (UpnpClient *)Cookie;
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

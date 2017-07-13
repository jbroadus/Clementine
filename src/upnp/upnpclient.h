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

#ifndef UPNPCLIENT_H
#define UPNPCLIENT_H

#include <QObject>
#include <upnp/upnp.h>

#include <upnp/upnpdatatypes.h>

class UpnpDescDoc;
class UpnpDescElement;
class UpnpDeviceInfo;
class UpnpManager;
class UpnpManagerPriv;

class UpnpClient : public QObject {
  Q_OBJECT

 public:
  UpnpClient(UpnpManagerPriv *priv);

  void SetMgr(UpnpManager *mgr);

 signals:
  void AddDevice(UpnpDeviceInfo *device);

 protected:
  bool StartAsyncSearch();

  /* Event Callbacks */
  static int EventCallback(Upnp_EventType EventType, void *Event, void *Cookie);
  int DiscoveryCallback(Upnp_EventType EventType,
                        struct Upnp_Discovery *discovery);

  /* XML parsing helpers */
  void ParseDeviceDesc(UpnpDescDoc &doc);
  void GetServicesFromDesc(UpnpDescDoc &doc, UpnpDeviceInfo *devInfo);

  void DownloadSpcd(UpnpServiceInfo *service);
  void ParseSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service);
  void GetActionArgsFromSpcd(UpnpDescElement &elem, UpnpActionInfo *action,
                             UpnpServiceInfo *service);
  void GetActionsFromSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service);
  void GetStateTableFromSpcd(UpnpDescDoc &doc, UpnpServiceInfo *service);

 protected:
  UpnpManager *mgr_;
  UpnpManagerPriv *priv_;
  UpnpClient_Handle handle;
};

#endif // UPNPCLIENT_H

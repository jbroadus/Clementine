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

#ifndef UPNPCLIENT_H
#define UPNPCLIENT_H

#include <memory>
#include <QObject>
#include <QMap>
#include <QMutex>
#include <QUrl>

#include <upnp/upnp.h>

// Wrapper for IXML_Document
class UpnpDescDoc;
// Wrapper for IXML_Element
class UpnpDescElement;

class UpnpDeviceInfo;
class UpnpServiceInfo;
class UpnpActionInfo;

class UpnpClient : public QObject {
  Q_OBJECT

public:
  UpnpClient();
  ~UpnpClient() {}
  bool Start();
  QStringList GetDeviceIDs();
  std::shared_ptr<UpnpDeviceInfo> GetDevice(const QString& id);
  std::shared_ptr<UpnpDeviceInfo> GetDevice(const QUrl& url);
  bool SendAction(UpnpActionInfo *action, UpnpServiceInfo *service);
signals:
  void DeviceDiscovered(const QString &location);

protected:
  static int ClientEventCallback(Upnp_EventType EventType, const void *Event,
                                 void *Cookie);

  bool StartAsyncSearch();
private:
  UpnpClient_Handle client_handle_;
  QMutex mutex_;
  QMap<QString, std::shared_ptr<UpnpDeviceInfo>> devices_;

  UpnpDeviceInfo *MakeDeviceInfo(const QUrl& location);

  /* Event Callbacks */
  int DiscoveryCallback(Upnp_EventType event_type,
                        UpnpDiscovery *discovery);
};

#endif  // UPNPCLIENT_H

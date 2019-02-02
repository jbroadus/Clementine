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

#ifndef DEVICES_UPNPDEVICE_H_
#define DEVICES_UPNPDEVICE_H_

#include "connecteddevice.h"

class UpnpClient;
class UpnpActionInfo;
class UpnpDeviceInfo;
class UpnpServiceInfo;

class UpnpDevice : public ConnectedDevice {
  Q_OBJECT
 public:
  Q_INVOKABLE UpnpDevice(const QUrl& url, DeviceLister* lister,
                         const QString& unique_id, DeviceManager* manager,
                         Application* app, int database_id, bool first_time);
  ~UpnpDevice();

  void Init();
  void ConnectAsync();
  bool CopyToStorage(const MusicStorage::CopyJob&) { return false; }
  bool DeleteFromStorage(const MusicStorage::DeleteJob&) { return false; }

  static QStringList url_schemes() {
    return QStringList() << "http"
                         << "https";
  }

 private:
  void Connect();
  bool GetBrowseAction();
  void Browse();

  UpnpClient *client_;
  std::shared_ptr<UpnpDeviceInfo> device_info_;
  UpnpServiceInfo *directory_service_;
  UpnpActionInfo *browse_action_;
};

#endif  // DEVICES_UPNPDEVICE_H_

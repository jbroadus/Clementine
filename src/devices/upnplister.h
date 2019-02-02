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

#ifndef DEVICES_UPNPLISTER_H_
#define DEVICES_UPNPLISTER_H_

#include "devicelister.h"

class UpnpClient;
class UpnpDeviceInfo;

class UpnpLister : public DeviceLister {
  Q_OBJECT

 public:
  UpnpLister(UpnpClient *client);
  ~UpnpLister();

  QStringList DeviceUniqueIDs();
  QVariantList DeviceIcons(const QString& id);
  QString DeviceManufacturer(const QString& id);
  QString DeviceModel(const QString& id);
  quint64 DeviceCapacity(const QString& id);
  quint64 DeviceFreeSpace(const QString& id);
  QVariantMap DeviceHardwareInfo(const QString& id);
  QString MakeFriendlyName(const QString& id);
  QList<QUrl> MakeDeviceUrls(const QString& id);
  void UnmountDevice(const QString& id);

 public slots:
  void UpdateDeviceFreeSpace(const QString& id);
  void DeviceDiscovered(const QString& id);

 protected:
  void Init();

 private:
  UpnpClient *client_;
};

#endif  // DEVICES_UPNPLISTER_H_

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

#include "upnplister.h"
#include "upnpclient.h"
#include "upnpdeviceinfo.h"
#include "core/logging.h"

#include <QStringList>

UpnpLister::UpnpLister(UpnpClient *client)
{
  client_ = client;
}

UpnpLister::~UpnpLister()
{
}

void UpnpLister::Init()
{
  connect(client_,
          SIGNAL(DeviceDiscovered(const QString &)),
          SLOT(DeviceDiscovered(const QString &)));
  client_->Start();
}

void UpnpLister::DeviceDiscovered(const QString& id)
{
  qLog(Info) << "Add " << id;
  emit DeviceAdded(id);
}

QStringList UpnpLister::DeviceUniqueIDs()
{
  return client_->GetDeviceIDs();
}

QVariantList UpnpLister::DeviceIcons(const QString& id)
{
  return QVariantList();
}

QString UpnpLister::DeviceManufacturer(const QString& id)
{
  return "";
}

QString UpnpLister::DeviceModel(const QString& id)
{
  return "";
}

quint64 UpnpLister::DeviceCapacity(const QString& id)
{
  return 0;
}

quint64 UpnpLister::DeviceFreeSpace(const QString& id)
{
  return 0;
}

QVariantMap UpnpLister::DeviceHardwareInfo(const QString& id)
{
  QVariantMap map;
  std::shared_ptr<UpnpDeviceInfo> device = client_->GetDevice(id);
  if (device) {
    map[QT_TR_NOOP("UDN")] = device->udn_;
    map[QT_TR_NOOP("Type")] = device->type_;
    map[QT_TR_NOOP("URL")] = device->url_;
  }
  return map;
}

QString UpnpLister::MakeFriendlyName(const QString& id)
{
  std::shared_ptr<UpnpDeviceInfo> device = client_->GetDevice(id);
  if (device)
    return device->name_;
  else
    return QString();
}

QList<QUrl> UpnpLister::MakeDeviceUrls(const QString& id)
{
  QList<QUrl> ret;
  std::shared_ptr<UpnpDeviceInfo> device = client_->GetDevice(id);
  if (device) {
    ret << QUrl(device->url_);
    qLog(Info) << ret[0];
  }
  return ret;
}

void UpnpLister::UnmountDevice(const QString& id)
{
}

void UpnpLister::UpdateDeviceFreeSpace(const QString& id)
{
}


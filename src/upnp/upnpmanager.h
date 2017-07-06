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

#ifndef UPNPMANAGER_H
#define UPNPMANAGER_H

#include "core/simpletreemodel.h"
#include "core/simpletreeitem.h"

class Application;
class UpnpManagerPriv;

struct UpnpServiceInfo {
  QString type;
};

struct UpnpDeviceInfo {
  QString udn;
  QString name;
  QString type;
  QList<UpnpServiceInfo> services;
};

class UpnpItem : public SimpleTreeItem<UpnpItem>
{
 public:
  enum Type {
    Upnp_Root,
    Upnp_Device,
    Upnp_Service,
    Upnp_Directory,
  };

 UpnpItem(SimpleTreeModel<UpnpItem>* model) :
  SimpleTreeItem<UpnpItem>(Upnp_Root, model) {}

 UpnpItem(Type type, UpnpItem *parent) :
  SimpleTreeItem<UpnpItem>(type, parent) {}
};

class UpnpDevice : public UpnpItem
{
 public:
 UpnpDevice(const UpnpDeviceInfo &info, UpnpItem *parent) :
  UpnpItem(Upnp_Device, parent),
    info_(info),
    has_media_server(false)
    {
      display_text = info.name;
      servicesItem_ = new UpnpItem(Upnp_Directory, this);
      servicesItem_->display_text = "services";
    }
  UpnpDeviceInfo info_;
  bool has_media_server;

  UpnpItem *servicesItem_;
};

class UpnpService : public UpnpItem
{
 public:
 UpnpService(UpnpServiceInfo &info, UpnpItem *parent) :
  UpnpItem(Upnp_Service, parent),
    info_(info) { display_text = info.type; }
  UpnpServiceInfo &info_;
};

class UpnpManager : public SimpleTreeModel<UpnpItem>
{
  Q_OBJECT

 public:
  UpnpManager(Application* app, QObject* parent = nullptr);
  ~UpnpManager();

  QVariant data(const QModelIndex& index, int role) const;

 signals:
  void DeviceDiscovered(int row);

 protected:
  void LazyPopulate(UpnpItem *item);

 private:
  friend class UpnpManagerPriv;
  int FindDeviceByUdn(const QString& udn) const;

 public slots:
  void AddDevice(const UpnpDeviceInfo &info);

 private:
  void AddService(UpnpServiceInfo &info, UpnpDevice *dev);
  
  Application* app_;
  UpnpManagerPriv *priv_;
};

#endif  // UPNPMANAGER_H

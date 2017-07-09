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

struct UpnpStateVarInfo {
  QString name;
  typedef enum {TYPE_I4, TYPE_UI4, TYPE_STR} datatype_t;
  datatype_t dataType;
  bool sendEvents;
};
typedef QList<UpnpStateVarInfo> UpnpStateVarList;

struct UpnpActionArgInfo {
  QString name;
  QString relStateVar;
  typedef enum {DIR_IN, DIR_OUT} direction_t;
  direction_t direction;
};
typedef QList<UpnpActionArgInfo> UpnpActionArgList;

struct UpnpActionInfo {
  QString name;
  UpnpActionArgList args;
};
typedef QList<UpnpActionInfo> UpnpActionList;

struct UpnpServiceInfo {
  QString type;
  QString id;
  QString scpdUrl;
  QString controlUrl;
  QString eventSubUrl;
  UpnpActionList actions;
  UpnpStateVarList stateVars;
  UpnpActionInfo *FindActionByName(QString &name) {
    UpnpActionList::iterator i;
    for (i = actions.begin(); i != actions.end(); i++) {
      if (i->name == name)
        return &(*i); 
    }
    return NULL;
  }
};
typedef QList<UpnpServiceInfo> UpnpServiceList;

struct UpnpDeviceInfo {
  QString udn;
  QString name;
  QString type;
  UpnpServiceList services;
  UpnpServiceInfo *FindServiceById(QString &id) {
    UpnpServiceList::iterator i;
    for (i = services.begin(); i != services.end(); i++) {
      if (i->id == id)
        return &(*i); 
    }
    return NULL;
  }
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
    info_(info)
    {
      display_text = info.type;
      lazy_loaded = true;
    }
  UpnpServiceInfo &info_;
};

class UpnpManager : public SimpleTreeModel<UpnpItem>
{
  Q_OBJECT

 public:
  UpnpManager(Application* app, QObject* parent = nullptr);
  ~UpnpManager();

  // QAbstractItemModel
  QVariant data(const QModelIndex& index, int role) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;

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

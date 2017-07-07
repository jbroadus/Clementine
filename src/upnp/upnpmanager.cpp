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

#include "upnpmanager.h"

#include "core/logging.h"
#include "upnppriv.h"



UpnpManager::UpnpManager(Application* app, QObject* parent)
  : SimpleTreeModel<UpnpItem>(new UpnpItem(this), parent),
  app_(app)
{
  priv_ = new UpnpManagerPriv();

  /* This signal will come from a non QT thread. */
  if (!connect(priv_, SIGNAL(AddDevice(const UpnpDeviceInfo &)),
               SLOT(AddDevice(const UpnpDeviceInfo &)),
               Qt::BlockingQueuedConnection))
    qLog(Error) << "Connection failed";

  priv_->SetMgr(this);
}

UpnpManager::~UpnpManager()
{
  delete priv_;
}

int UpnpManager::FindDeviceByUdn(const QString& udn) const {
  for (int i = 0; i < root_->children.count(); ++i) {
    UpnpDevice *dev = static_cast<UpnpDevice *>(root_->children[i]);
    Q_ASSERT(dev);
    if (dev->info_.udn == udn) return i;
  }
  return -1;
}

void UpnpManager::LazyPopulate(UpnpItem *item)
{
  if (item->type == UpnpItem::Upnp_Root) {
    
  }
  return;
}

void UpnpManager::AddDevice(const UpnpDeviceInfo &info)
{
  int idx = FindDeviceByUdn(info.udn);

  if (idx != -1) {
    return;
  }

  qLog(Debug) << "New device";
  qLog(Debug) << info.udn;
  qLog(Debug) << info.name;
  qLog(Debug) << info.type;

  beginInsertRows(ItemToIndex(root_), root_->children.count(),
                    root_->children.count());
  UpnpDevice *dev = new UpnpDevice(info, root_);
  endInsertRows();

  beginInsertRows(ItemToIndex(dev->servicesItem_), dev->servicesItem_->children.count(),
                    dev->servicesItem_->children.count());
  for (int i=0; i<dev->info_.services.count(); i++) {
    AddService(dev->info_.services[i], dev); 
  }
  endInsertRows();
}

void UpnpManager::AddService(UpnpServiceInfo &info, UpnpDevice *dev)
{
  qLog(Debug) << "New service";
  qLog(Debug) << info.type;

  //if (info.type == "")

  new UpnpService(info, dev->servicesItem_);
}

QVariant UpnpManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0) return QVariant();

  const UpnpItem *item = IndexToItem(index);

  switch (role) {
  case Qt::DisplayRole: {
    return item->display_text;
  }
  default:
    return QVariant();
  }
}

Qt::ItemFlags UpnpManager::flags(const QModelIndex& index) const {
  const UpnpItem *item = IndexToItem(index);
  switch (item->type) {
  case UpnpItem::Upnp_Service:
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  default:
    return Qt::ItemIsEnabled;
  }
}

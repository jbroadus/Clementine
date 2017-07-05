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

#include <QAbstractListModel>

#include "library/librarymodel.h"

class UpnpManagerPriv;

class UpnpManager : public QAbstractListModel {
  Q_OBJECT

 public:
  UpnpManager(Application* app, QObject* parent = nullptr);
  ~UpnpManager();


  // QAbstractListModel
  int rowCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;

 signals:
  void DeviceDiscovered(int row);

 private:
  friend class UpnpManagerPriv;
  struct UpnpDevice {
    QString udn;
    QString name;
    QString type;
  };
  void AddDevice(UpnpDevice &dev);
  int FindDeviceByUdn(const QString& udn) const;

 private:
  
  Application* app_;
  UpnpManagerPriv *priv_;
  QList<UpnpDevice> devices_;
};

#endif  // UPNPMANAGER_H

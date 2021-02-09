/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#ifndef UPNPROOT_H
#define UPNPROOT_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QXmlStreamWriter>
#include <upnp/upnp.h>
#include <memory>

class UpnpManager;
class UpnpService;

namespace Clementine {
  class UpnpActionRequest;
  class UpnpSubscriptionRequest;
};

class DeviceRegistration : public QObject {
  Q_OBJECT

 public:
  DeviceRegistration(const QString& descUrl, QObject* parent = nullptr);
  ~DeviceRegistration();
  bool Register();

 signals:
  void ActionRequest(const Clementine::UpnpActionRequest& req);
  void SubscriptionRequest(const Clementine::UpnpSubscriptionRequest& req);

 protected:
  int Callback(Upnp_EventType type, const void* event);

 private:
  // Calls the instance method
  static int SCallback(Upnp_EventType type, const void* event, void* cookie);

  // The upnp system holds on to the const data pointer, so must be const.
  const QByteArray descUrl_;

  UpnpDevice_Handle handle_;

  // Handle is valid
  bool registered_;
};

class UpnpRoot : public QObject {
  Q_OBJECT
 public:
  UpnpRoot(UpnpManager* parent);

  bool Register();
  bool Unregister();

  UpnpManager* manager() { return manager_; }

  typedef QList<UpnpService*> ServiceList;
  const ServiceList serviceList() { return serviceMap_.values(); }

 private slots:
  void ActionRequest(const Clementine::UpnpActionRequest& req);
  void SubscriptionRequest(const Clementine::UpnpSubscriptionRequest& req);

 private:
  class DescWriter : public QXmlStreamWriter {
   public:
    DescWriter(UpnpRoot* root, QByteArray* desc);

   private:
    void writeDesc();
    void writeServiceList();
    UpnpRoot* root_;
  };

  void AddService(UpnpService* service);

  UpnpManager* manager_;
  QHash<QString, UpnpService*> serviceMap_;
  std::unique_ptr<DeviceRegistration> registration_;
};

#endif  // UPNPROOT_H

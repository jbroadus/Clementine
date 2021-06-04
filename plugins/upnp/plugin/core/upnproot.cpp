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

#include "upnproot.h"

#include <upnp/upnptools.h>

#include <QXmlStreamWriter>

#include "core/logging.h"
#include "services/avtransport.h"
#include "services/connectionmanager.h"
#include "services/renderingcontrol.h"
#include "upnpevents.h"
#include "upnpmanager.h"
#include "upnpserver.h"

DeviceRegistration::DeviceRegistration(const QString& descUrl, QObject* parent)
    : QObject(parent), descUrl_(descUrl.toUtf8()), registered_(false) {}

bool DeviceRegistration::Register() {
  int ret = UpnpRegisterRootDevice2(UPNPREG_URL_DESC, descUrl_.constData(), 0,
                                    0, SCallback, this, &handle_);
  if (ret != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpRegisterRootDevice2 error" << UpnpGetErrorMessage(ret);
    return false;
  }
  registered_ = true;
  qLog(Debug) << "Registered";

  qLog(Debug) << "Advertising...";
  // Using default expiration. This continues to advertise as long as the
  // device is registered.
  ret = UpnpSendAdvertisement(handle_, 0);
  if (ret != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpSendAdvertisement error" << UpnpGetErrorMessage(ret);
    return false;
  }

  return true;
}

DeviceRegistration::~DeviceRegistration() {
  if (registered_) {
    UpnpUnRegisterRootDevice(handle_);
  }
}

int DeviceRegistration::SCallback(Upnp_EventType type, const void* event,
                                  void* cookie) {
  DeviceRegistration* registration = static_cast<DeviceRegistration*>(cookie);
  if (registration == nullptr) {
    qLog(Error) << "Callback error";
    return -1;
  }
  return registration->Callback(type, event);
}

int DeviceRegistration::Callback(Upnp_EventType type, const void* event) {
  switch (type) {
    case UPNP_CONTROL_ACTION_REQUEST: {
      Clementine::UpnpActionRequest req(event);
      // Blocking
      emit ActionRequest(&req);
    } break;
    default:
      qLog(Debug) << "Unhandled event" << type;
  }
  return 0;
}

void UpnpRoot::ActionRequest(Clementine::UpnpActionRequest* req) {
  UpnpService* service = serviceMap_.value(req->GetServiceId(), nullptr);
  if (service == nullptr) {
    qLog(Warning) << "Received action for unknown service"
                  << req->GetServiceId();
    req->SetErrorCode(401);
  }
  service->ActionRequest(req);
}

void UpnpRoot::SubscriptionRequest(Clementine::UpnpSubscriptionRequest* req) {}

UpnpRoot::UpnpRoot(UpnpManager* parent) : QObject(parent), manager_(parent) {
  AddService(new AVTransport(manager_, this));
  AddService(new ConnectionManager(manager_, this));
  AddService(new RenderingControl(manager_, this));
}

void UpnpRoot::AddService(UpnpService* service) {
  serviceMap_[service->id()] = service;
}

bool UpnpRoot::Register() {
  for (UpnpService* service : serviceList()) {
    service->WriteDesc();
  }

  QByteArray desc;
  DescWriter writer(this, &desc);
  manager_->GetServer()->AddFile("/desc.xml", desc);

  QString url = manager_->GetServer()->GetUrl("/desc.xml");
  registration_.reset(new DeviceRegistration(url));
  connect(registration_.get(),
          SIGNAL(ActionRequest(Clementine::UpnpActionRequest*)),
          SLOT(ActionRequest(Clementine::UpnpActionRequest*)),
          Qt::BlockingQueuedConnection);
  registration_->Register();
  return true;
}

bool UpnpRoot::Unregister() {
  registration_.reset();
  return true;
}

UpnpRoot::DescWriter::DescWriter(UpnpRoot* root, QByteArray* desc)
    : QXmlStreamWriter(desc), root_(root) {
  setAutoFormatting(true);
  writeDesc();
}

void UpnpRoot::DescWriter::writeDesc() {
  QString xmlns = "urn:schemas-upnp-org:device-1-0";
  QString deviceType = "urn:schemas-upnp-org:device:MediaRenderer:1";
  QString udn = "uuid:d1b5e0fd-67d0-45fb-b235-add8d942b687";
  QString friendlyName = "Clementine";
  writeStartDocument();
  writeStartElement("root");
  writeAttribute("xmlns", xmlns);

  writeStartElement("specVersion");
  writeTextElement("major", "1");
  writeTextElement("minor", "0");
  writeEndElement();  // specVersion

  writeStartElement("device");
  writeTextElement("deviceType", deviceType);
  writeTextElement("friendlyName", friendlyName);
  writeTextElement("UDN", udn);

  writeServiceList();

  writeEndElement();  // device

  writeEndElement();  // root
  writeEndDocument();
}

void UpnpRoot::DescWriter::writeServiceList() {
  writeStartElement("serviceList");
  for (UpnpService* service : root_->serviceList()) {
    service->FillServiceEntry(this);
  }
  writeEndElement();  // serviceList
}

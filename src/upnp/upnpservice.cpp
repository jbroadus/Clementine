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

#include "upnpservice.h"

#include "core/logging.h"
#include "scpdbuilder.h"
#include "scpdparser.h"
#include "upnpevents.h"
#include "upnpmanager.h"
#include "upnpserver.h"

UpnpService::UpnpService(const QString& name, int rev, UpnpManager* manager, QObject* parent)
  : QObject(parent),
    name_(name),
    type_(QString("urn:schemas-upnp-org:service:%1:1").arg(name)),
    id_(QString("urn:upnp-org:serviceId:%1").arg(name)),
    rev_(rev),
    manager_(manager) {}

void UpnpService::AddAction(UpnpAction&& action) {
  actionMap_[action.name_] = action;
}

void UpnpService::ActionRequest(Clementine::UpnpActionRequest req) {
  const UpnpAction& action = actionMap_.value(req.GetActionName());
  qLog(Debug) << name_ << "got action" << action.name_;
  req.InitResponse(type_);
}

bool UpnpService::FillServiceEntry(QXmlStreamWriter* writer) {
  writer->writeStartElement("service");
  writer->writeTextElement("serviceType", type_);
  writer->writeTextElement("serviceId", id_);
  writer->writeTextElement("SCPDURL", ScpdUrl());
  writer->writeTextElement("controlURL", ControlUrl());
  writer->writeTextElement("eventSubURL", EventUrl());
  writer->writeEndElement(); // service
  return true;
}

UpnpAction::UpnpAction(const QString& name, bool defaults) : name_(name) {
  if (defaults) {
    arg("InstanceID", "A_ARG_TYPE_InstanceID", UpnpAction::Arg::IN);
  }
}

void UpnpAction::UpnpAction::arg(const QString& name, const QString& varName, UpnpAction::Arg::Dir dir) {
  args_ << UpnpAction::Arg(name, varName, dir);
}

bool UpnpService::ParseDesc(const QString& fileName) {
  QFile desc(fileName);
  if (!desc.open(QIODevice::ReadOnly)) {
    qLog(Error) << "Couldn't load" << fileName;
    return false;
  }

  ScpdParser parser(this, &desc);
  return parser.parse();
}

bool UpnpService::WriteDesc() {
  QByteArray desc;
  ScpdBuilder builder(this, &desc);
  if (!builder.build()) {
    return false;
  }
  manager_->GetServer()->AddFile(ScpdUrl(), desc);
  return true;
}


UpnpServiceHosted::UpnpServiceHosted(const QString& name, int rev, UpnpManager* manager, QObject* parent)
  : UpnpService(name, rev, manager, parent) {}

bool UpnpServiceHosted::WriteDesc() {
  return manager_->GetServer()->AddFile(ScpdUrl(), SrcPath());
}

QString UpnpServiceHosted::SrcPath() {
  return QString(":/upnp/%1%2.xml").arg(name_).arg(rev_);
}

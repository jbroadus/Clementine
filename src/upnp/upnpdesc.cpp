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

#include "upnpdesc.h"

#include <QDir>
#include <QUrl>
#include <QXmlStreamWriter>

#include "upnpmanager.h"
#include "core/logging.h"

#define DESC_FILE "desc.xml"

UpnpDesc::UpnpDesc(QString &webdir, QString &dir, UpnpDeviceInfo &info) :
  webdir_(webdir)
{
  path_= dir + "/" + DESC_FILE;
  CreateDesc(info);
}

UpnpDesc::UpnpDesc(QString &webdir, UpnpServiceInfo &info) :
  webdir_(webdir)
{
  QUrl url(info.scpdUrl);
  path_= url.path();
  CreateSCPD(info);
}

QString UpnpDesc::GetUrlPath()
{
  return path_;
}

bool UpnpDesc::Open()
{
  QString fullPath = QDir::toNativeSeparators(webdir_ + "/" + path_);
  QFileInfo info(fullPath);
  QString dirPath = info.path();

  if (!QFile::exists(dirPath)) {
    QDir dir;
    if (!dir.mkpath(dirPath)) {
      qLog(Error) << "Could not create " << dirPath;
      return false;
    }
  }

  file_.setFileName(fullPath);

  if (!file_.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
      qLog(Error) << "Could not create " << fullPath;
      return false;
  }

  return true;
}


bool UpnpDesc::AddSpecVersion(QXmlStreamWriter &out)
{
  out.writeStartElement("specVersion");
  out.writeTextElement("major", "1");
  out.writeTextElement("minor", "0");
  out.writeEndElement(); // specVersion
  return true;
}

bool UpnpDesc::GenerateDesc(QXmlStreamWriter &out, UpnpDeviceInfo &info)
{
  out.setAutoFormatting(true);
  out.writeStartDocument();
  out.writeStartElement("root");

  AddSpecVersion(out);
  out.writeStartElement("device");
  out.writeTextElement("deviceType", info.type);
  out.writeTextElement("friendlyName", info.name);
  out.writeTextElement("UDN", info.udn);

  out.writeStartElement("serviceList");
  UpnpServiceList::iterator i;
  for (i = info.services.begin(); i != info.services.end(); i++) {
    out.writeStartElement("service");
    out.writeTextElement("serviceType", i->type);
    out.writeTextElement("serviceId", i->id);
    out.writeTextElement("SCPDURL", i->scpdUrl);
    out.writeTextElement("controlURL", i->controlUrl);
    out.writeTextElement("eventSubURL", i->eventSubUrl);
    out.writeEndElement(); // service
  }
  out.writeEndElement(); // serviceList

  out.writeEndElement(); // device

  out.writeEndElement(); // root
  out.writeEndDocument();

  return true;
}

bool UpnpDesc::CreateDesc(UpnpDeviceInfo &info)
{
  if (!Open())
    return false;

  QXmlStreamWriter out(&file_);
  GenerateDesc(out, info);
  file_.flush();

  return true;
}

bool UpnpDesc::GenerateSCPD(QXmlStreamWriter &out, UpnpServiceInfo &info)
{
  out.setAutoFormatting(true);
  out.writeStartDocument();
  out.writeStartElement("scpd");

  AddSpecVersion(out);

  // actionList
  out.writeStartElement("actionList");
  UpnpActionList::iterator action_itr;
  UpnpActionArgList::iterator arg_itr;
  for (action_itr = info.actions.begin(); action_itr != info.actions.end(); action_itr++) {
    out.writeStartElement("action");
    out.writeTextElement("name", action_itr->name);

    out.writeStartElement("argumentList");
    /* Input arguments */
    for (arg_itr = action_itr->in_args.begin();
         arg_itr != action_itr->in_args.end();
         arg_itr++) {
      out.writeStartElement("argument");
      out.writeTextElement("name", arg_itr->name);
      out.writeTextElement("direction", "in");
      if (arg_itr->relStateVar)
        out.writeTextElement("relatedStateVariable",
                             arg_itr->relStateVar->name);
      out.writeEndElement(); // argument
    }
    /* Output arguments */
    for (arg_itr = action_itr->out_args.begin();
         arg_itr != action_itr->out_args.end();
         arg_itr++) {
      out.writeStartElement("argument");
      out.writeTextElement("name", arg_itr->name);
      out.writeTextElement("direction", "out");
      if (arg_itr->relStateVar)
        out.writeTextElement("relatedStateVariable",
                             arg_itr->relStateVar->name);
      out.writeEndElement(); // argument
    }
    out.writeEndElement(); // ArgumentList
    out.writeEndElement(); // action
  }
  out.writeEndElement(); // actionList

  // serviceStateTable
  out.writeStartElement("serviceStateTable");
  UpnpStateVarList::iterator stateVar_itr;
  for (stateVar_itr = info.stateVars.begin(); stateVar_itr != info.stateVars.end(); stateVar_itr++) {
    out.writeStartElement("stateVariable");
    out.writeAttribute("sendEvents", stateVar_itr->sendEvents ? "yes" : "no");
    out.writeTextElement("name", stateVar_itr->name);

    const char *dataTypeStr;
    switch(stateVar_itr->dataType) {
    case UpnpStateVarInfo::TYPE_I4:
      dataTypeStr = "i4";
      break;
    case UpnpStateVarInfo::TYPE_UI4:
      dataTypeStr = "ui4";
      break;
    case UpnpStateVarInfo::TYPE_STR:
      dataTypeStr = "string";
      break;
    default:
      dataTypeStr = "unknown";
      break;
    }
    out.writeTextElement("dataType", dataTypeStr);
    out.writeEndElement(); // stateVariable
  }
  out.writeEndElement(); // serviceStateTable

  out.writeEndElement(); // scpd
  out.writeEndDocument();

  return true;
}

bool UpnpDesc::CreateSCPD(UpnpServiceInfo &info)
{
  if (!Open())
    return false;

  QXmlStreamWriter out(&file_);
  GenerateSCPD(out, info);
  file_.flush();

  return true;
}

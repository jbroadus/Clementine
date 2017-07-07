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
  for (int i=0; i<info.services.count(); i++) {
    out.writeStartElement("service");
    out.writeTextElement("serviceType", info.services[i].type);
    out.writeTextElement("serviceId", info.services[i].id);
    out.writeTextElement("SCPDURL", info.services[i].scpdUrl);
    out.writeTextElement("controlURL", info.services[i].controlUrl);
    out.writeTextElement("eventSubURL", info.services[i].eventSubUrl);
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

bool UpnpDesc::GenerateSCPD(QXmlStreamWriter &out)
{
  out.setAutoFormatting(true);
  out.writeStartDocument();
  out.writeStartElement("scpd");

  AddSpecVersion(out);

  out.writeEndElement(); // scpd
  out.writeEndDocument();

  return true;
}

bool UpnpDesc::CreateSCPD(UpnpServiceInfo &info)
{
  if (!Open())
    return false;

  QXmlStreamWriter out(&file_);
  GenerateSCPD(out);
  file_.flush();

  return true;
}

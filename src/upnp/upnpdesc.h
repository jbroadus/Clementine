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

#ifndef UPNPDESC_H
#define UPNPDESC_H

#include <QFile>
#include <QString>

struct UpnpDeviceInfo;
struct UpnpServiceInfo;

class QXmlStreamWriter;

class UpnpDesc
{
 public:
  UpnpDesc(QString &webdir, QString &dir, UpnpDeviceInfo &info);
  UpnpDesc(QString &webdir, UpnpServiceInfo &info);

  QString GetUrlPath();
 private:
  /* XML generation */
  bool Open();
  bool AddSpecVersion(QXmlStreamWriter &out);

  bool GenerateDesc(QXmlStreamWriter &out, UpnpDeviceInfo &info);
  bool CreateDesc(UpnpDeviceInfo &info);

  bool GenerateSCPD(QXmlStreamWriter &out);
  bool CreateSCPD(UpnpServiceInfo &info);

  QString webdir_;
  QString path_;
  QFile file_;
};

#endif  // UPNPDESC_H

/* This file is part of Clementine.
   Copyright 2019, Jim Broadus <jbroadus@gmail.com>

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

#ifndef UPNPDEVICEINFO_H
#define UPNPDEVICEINFO_H

#include <QList>
#include <QString>
#include <QUrl>

class UpnpDescDoc;
class UpnpDescElement;
class UpnpServiceInfo;

class UpnpDeviceInfo {
 public:
  static const char *tag_serviceList;
  static const char *tag_service;

  bool Populate(UpnpDescDoc &doc);
  void GetServicesFromDesc(UpnpDescDoc &doc);
  UpnpServiceInfo *AddService(UpnpServiceInfo &service);
  UpnpServiceInfo *AddService(UpnpDescElement &element);

  UpnpServiceInfo *FindServiceById(QString &id);

  QString udn_;
  QUrl url_;
  QString name_;
  QString type_;
  QList<UpnpServiceInfo> services_;
};

#endif /* UPNPDEVICEINFO_H */

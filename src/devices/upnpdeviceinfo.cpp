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

#include "upnpdeviceinfo.h"
#include "upnpserviceinfo.h"
#include "upnpdoc.h"

const char *UpnpDeviceInfo::tag_serviceList = "serviceList";
const char *UpnpDeviceInfo::tag_service = "service";

bool UpnpDeviceInfo::Populate(UpnpDescDoc &doc)
{
  if (!doc.GetNodeStr(udn_, "UDN"))
    return false;

  doc.GetNodeStr(name_, "friendlyName");
  doc.GetNodeStr(type_, "deviceType");
  GetServicesFromDesc(doc);
  return true;
}

void UpnpDeviceInfo::GetServicesFromDesc(UpnpDescDoc &doc)
{
  UpnpElementList list(doc, "serviceList", "service");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    AddService(elem);
  }
}

UpnpServiceInfo *UpnpDeviceInfo::FindServiceById(QString &id) {
  QList<UpnpServiceInfo>::iterator i;
  for (i = services_.begin(); i != services_.end(); i++) {
    if (i->id_ == id)
      return &(*i); 
  }
  return NULL;
}

UpnpServiceInfo *UpnpDeviceInfo::AddService(UpnpServiceInfo& service)
{
  services_ << service;
  return &services_.last();
}

UpnpServiceInfo *UpnpDeviceInfo::AddService(UpnpDescElement& element)
{
  UpnpServiceInfo service;
  if (!service.Populate(element))
    return NULL;
  return AddService(service);
}


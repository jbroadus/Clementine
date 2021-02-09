#include "upnpevents.h"

#include <upnp/upnp.h>

// Conversion helper
static QString GetString(const UpnpString *p) {
  return QString(UpnpString_get_String(p));
}

QString Clementine::UpnpActionRequest::GetServiceId() const {
  return GetString(UpnpActionRequest_get_ServiceID(req_));
}

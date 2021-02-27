#include "upnpevents.h"

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#if 0
// Conversion helper
static QString GetString(const UpnpString *p) {
  return QString(UpnpString_get_String(p));
}
#endif

QString Clementine::UpnpActionRequest::GetServiceId() const {
  return UpnpActionRequest_get_ServiceID_cstr(req_);
}

QString Clementine::UpnpActionRequest::GetActionName() const {
  return UpnpActionRequest_get_ActionName_cstr(req_);
}

void Clementine::UpnpActionRequest::InitResponse(const QString& serviceType) {
  UpnpMakeActionResponse(UpnpActionRequest_get_ActionName_cstr(req_),
                         serviceType.toUtf8().constData(), 0, nullptr);
                         
}

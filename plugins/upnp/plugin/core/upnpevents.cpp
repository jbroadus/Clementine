#include "upnpevents.h"

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "core/logging.h"

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
  if (resp_ != nullptr) {
    qLog(Error) << "Response already exists.";
    return;
  }
  resp_ = UpnpMakeActionResponse(UpnpActionRequest_get_ActionName_cstr(req_),
                                 serviceType.toUtf8().constData(), 0, nullptr);
  UpnpActionRequest_set_ActionResult(req_, resp_);
}

void Clementine::UpnpActionRequest::SetErrorCode(int code) {
  UpnpActionRequest_set_ErrCode(req_, code);
}

int Clementine::UpnpActionRequest::GetErrorCode() {
  return UpnpActionRequest_get_ErrCode(req_);
}

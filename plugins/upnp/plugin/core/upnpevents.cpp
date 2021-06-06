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

void Clementine::UpnpActionRequest::Parse() {
   IXML_Document* doc = UpnpActionRequest_get_ActionRequest(req_);
   IXML_Node* child = ixmlNode_getFirstChild(&(doc->n));
   child = ixmlNode_getFirstChild(child);
   while(child) {
     DOMString str = ixmlPrintNode(child);
     qLog(Debug) << str;
     ixmlFreeDOMString(str);
     child = ixmlNode_getNextSibling(child);
   }
}

QString Clementine::UpnpActionRequest::GetServiceId() const {
  return UpnpActionRequest_get_ServiceID_cstr(req_);
}

QString Clementine::UpnpActionRequest::GetActionName() const {
  return UpnpActionRequest_get_ActionName_cstr(req_);
}

void Clementine::UpnpActionRequest::InitResponse(const QString& service_type) {
  if (resp_ != nullptr) {
    qLog(Error) << "Response already exists.";
    return;
  }
  service_type_ = service_type;
  resp_ = UpnpMakeActionResponse(UpnpActionRequest_get_ActionName_cstr(req_),
                                 service_type_.toUtf8().constData(), 0,
                                 nullptr);
  UpnpActionRequest_set_ActionResult(req_, resp_);
}

void Clementine::UpnpActionRequest::AddToResponse(const QString& name,
                                                  const QString& val) {
  UpnpAddToActionResponse(&resp_, UpnpActionRequest_get_ActionName_cstr(req_),
                          service_type_.toUtf8().constData(),
                          name.toUtf8().constData(),
                          val.toUtf8().constData());
}

void Clementine::UpnpActionRequest::SetErrorCode(int code) {
  UpnpActionRequest_set_ErrCode(req_, code);
}

int Clementine::UpnpActionRequest::GetErrorCode() {
  return UpnpActionRequest_get_ErrCode(req_);
}

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

#ifndef UPNPEVENTS_H
#define UPNPEVENTS_H

#include <QObject>
#include <memory>
#include <upnp/upnp.h>

// Use a namespace so names don't conflict with upnp library.
namespace Clementine {

  class UpnpActionRequest : public QObject {
    Q_OBJECT
  public:
    using Req = ::UpnpActionRequest;
    UpnpActionRequest() = default;
    UpnpActionRequest(const void* req) : req_((Req *)req), resp_(nullptr) {}
    UpnpActionRequest(const UpnpActionRequest& other) : req_(other.req_), resp_(other.resp_) {}

    QString GetServiceId() const;
    QString GetActionName() const;

    // Indicates that event was received
    void InitResponse(const QString& serviceType);

    // We don't own this memory;
    Req* req_;
    IXML_Document* resp_;
  };

  class UpnpSubscriptionRequest : public QObject {
    Q_OBJECT
  public:
    UpnpSubscriptionRequest(const void* req);
    ::UpnpSubscriptionRequest* req_;
  };
};

Q_DECLARE_METATYPE(Clementine::UpnpActionRequest);

#endif  // UPNPEVENTS_H

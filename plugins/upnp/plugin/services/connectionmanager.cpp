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

/*
  V1 spec: http://upnp.org/specs/av/UPnP-av-ConnectionManager-v1-Service.pdf
 */

#include "connectionmanager.h"

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/upnpmanager.h"

#include <QHostAddress>
#include <QNetworkInterface>

ConnectionManager::ConnectionManager(UpnpManager* manager, QObject* parent)
    : UpnpServiceHosted("ConnectionManager", 1, manager, parent) {
  Setup();
}

void ConnectionManager::Setup() {
  ParseDesc(SrcPath());

  // Initial state values.
  UpnpVar* var = GetVar("SinkProtocolInfo");
  if (var) {
    var->SetStringValue(GetSinkProtocolInfo());
  } else {
    qLog(Error) << "No variable SinkProtocolInfo";
  }
}

QString ConnectionManager::GetSinkProtocolInfo() {
  QString network = "*";
  QList<QHostAddress> addrs = QNetworkInterface::allAddresses();
  for (const QHostAddress& addr : addrs) {
    if (addr.isNull()) continue;
    if (addr.isLoopback()) continue;
    if (addr.protocol() != QAbstractSocket::IPv4Protocol) continue;
    network = addr.toString();
    break;
  }
  return QString("internal:%1:*:local-library").arg(network);
}

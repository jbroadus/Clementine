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

#include "upnpmanager.h"

#include "upnproot.h"
#include "upnpserver.h"
#include "core/application.h"
#include "core/logging.h"

#include <upnp/upnp.h>
#include <upnp/upnptools.h>
//#define UPNP_DEBUG
#ifdef UPNP_DEBUG
#include <upnp/upnpdebug.h>
#endif

UpnpManager::UpnpManager(Application* app)
  : QObject(app),
    app_(app),
    root_device_(new UpnpRoot(this)),
    http_server_(new UpnpServer(this)) {}

UpnpManager::~UpnpManager() {
  UpnpFinish();
}

void UpnpManager::Start() {
#ifdef UPNP_DEBUG
  UpnpSetLogFileNames(NULL, NULL);
  UpnpSetLogLevel(UPNP_ALL);
  UpnpInitLog();
#endif

  // Any interface, any port
  int ret = UpnpInit2(nullptr, 0);
  if (ret != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpInit2 error" << UpnpGetErrorMessage(ret);
    return;
  }

  qLog(Debug) << "Upnp using port" << UpnpGetServerPort();

  http_server_->Start();

  root_device_->Register();

}

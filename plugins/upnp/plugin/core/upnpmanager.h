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

#ifndef UPNPMANAGER_H
#define UPNPMANAGER_H

#include "interface/upnpmanager.h"
#include "interface/upnpplugin.h"

class Application;
class UpnpRoot;
class UpnpServer;

class UpnpManager : public IClementine::UpnpManager {
  Q_OBJECT

 public:
  UpnpManager(Application* app);
  ~UpnpManager();

  void Start();

  UpnpServer* GetServer() { return http_server_; }
  Application* GetApp() { return app_; };

 private:
  Application* app_;
  UpnpRoot* root_device_;
  UpnpServer* http_server_;
};

class UpnpPlugin : public QObject, public IClementine::UpnpPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.clementine-player.UpnpPlugin")
  Q_INTERFACES(IClementine::UpnpPlugin)
 public:
  UpnpPlugin();
  UpnpManager* MakeUpnpManager(Application* app);
};

#endif  // UPNPMANAGER_H

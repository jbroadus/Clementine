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

#include "plugin.h"

#include <QPluginLoader>

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "interfaces/player.h"
#include "interfaces/service.h"
#include "interfaces/settings.h"
#include "pluginmanager.h"

Plugin::Plugin(PluginManager* manager)
    : QObject(manager),
      service_(nullptr),
      player_(nullptr),
      settings_(nullptr),
      loader_(nullptr),
      mgr_(manager) {}

Plugin::~Plugin() {
  if (loader_) {
    loader_->unload();
    delete loader_;
  }
}

bool Plugin::Load(const QString& path) {
  loader_ = new QPluginLoader(path, this);
  if (!loader_->load()) {
    qLog(Error) << "Could not load module" << path << ":"
                << loader_->errorString();
    return false;
  }

  if (!AddInterfaces(loader_->metaData(), loader_->instance())) {
    return false;
  }

  return true;
}

bool Plugin::AddInterfaces(const QJsonObject& metaData, QObject* inst) {
  QString iid = metaData["IID"].toString();
  qLog(Debug) << "Try to add plugin" << iid;
  service_ = qobject_cast<IClementine::Service*>(inst);
  if (service_ == nullptr) {
    qLog(Debug) << "Not a service plugin";
    return false;
  }

  // Set up component interfaces.
  for (IClementine::ComponentInterface* interface : service_->GetInterfaces()) {
    ConnectComponent(interface);
  }

  return true;
}

void Plugin::ConnectComponent(IClementine::ComponentInterface* interface) {
  IClementine::Player* player = qobject_cast<IClementine::Player*>(interface);
  if (player) {
    ConnectPlayer(player);
    return;
  }

  IClementine::Settings* settings =
      qobject_cast<IClementine::Settings*>(interface);
  if (settings) {
    ConnectSettings(settings);
    return;
  }

  qLog(Warning) << "Unknown interface" << interface->GetName();
}

void Plugin::ConnectPlayer(IClementine::Player* interface) {
  qLog(Debug) << "Connecting player interface";
  player_ = interface;
  Player* player = mgr_->app()->player();
  connect(player, SIGNAL(Playing()), interface, SLOT(Playing()));
  connect(player, SIGNAL(Paused()), interface, SLOT(Paused()));
  connect(player, SIGNAL(Stopped()), interface, SLOT(Stopped()));

  connect(interface, SIGNAL(Play()), player, SLOT(Play()));
  connect(interface, SIGNAL(Pause()), player, SLOT(Pause()));
  connect(interface, SIGNAL(Stop()), player, SLOT(Stop()));
}

void Plugin::ConnectSettings(IClementine::Settings* interface) {
  qLog(Debug) << "Connecting settings interface";
  settings_ = interface;
}

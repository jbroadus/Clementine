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

#include "pluginmanager.h"

#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "core/utilities.h"
#include "interfaces/service.h"
#include "interfaces/player.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

extern void PluginHostInit();

PluginManager::PluginManager(Application* app, QObject* parent)
  : QObject(parent),
    app_(app) {
  InitPlugins();
}

PluginManager::~PluginManager() {
  for (QPluginLoader* loader : dynamicPlugins_) {
    loader->unload();
  }
}

void PluginManager::InitPlugins() {
  PluginHostInit();
  for (QStaticPlugin& plugin : QPluginLoader::staticPlugins()) {
    AddInterface(plugin.metaData(), plugin.instance());
  }
  FindPlugins(QDir(Utilities::GetConfigPath(Utilities::Path_Root)).filePath("plugins"));
}

void PluginManager::FindPlugins(const QString& path) {
  QDir dir(path);
  for (const QFileInfo& info : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
    if (info.isFile()) {
      LoadPlugin(info.absoluteFilePath());
    }
  }
}

bool PluginManager::LoadPlugin(const QString& path) {
  QPluginLoader* loader = new QPluginLoader(path, this);
  if (!loader->load()) {
    qLog(Error) << "Could not load module" << path << ":" << loader->errorString();
    delete loader;
    return false;
  }

  if (!AddInterface(loader->metaData(), loader->instance())) {
    loader->unload();
    delete loader;
    return false;
  }

  qLog(Info) << "Loaded plugin" << path;
  dynamicPlugins_ << loader;
  return true;
}

void PluginManager::StartAll() {
  for (IClementine::Service* service : services_) {
    service->Start();
  }
}

bool PluginManager::AddInterface(const QJsonObject& metaData, QObject* inst) {
  QString iid = metaData["IID"].toString();
  qLog(Debug) << "Try to add plugin" << iid;
  IClementine::Service* service =
    qobject_cast<IClementine::Service *>(inst);
  if (service == nullptr) {
    qLog(Debug) << "Not a service plugin";
    return false;
  }

  // Set up component interfaces.
  for (IClementine::ComponentInterface* interface : service->GetInterfaces()) {
    IClementine::Player* player = qobject_cast<IClementine::Player *>(interface);
    if (player) {
      ConnectPlayer(player);
    } else {
      qLog(Warning) << "Unknown interface" << interface->GetName();
    }
  }

  services_ << service;
  return true;
}

void PluginManager::ConnectPlayer(IClementine::Player* interface) {
  qLog(Debug) << "Connecting player interface";
  Player* player = app_->player();
  connect(player, SIGNAL(Playing()), interface, SLOT(Playing()));
  connect(player, SIGNAL(Paused()), interface, SLOT(Paused()));
  connect(player, SIGNAL(Stopped()), interface, SLOT(Stopped()));

  connect(interface, SIGNAL(Play()), player, SLOT(Play()));
  connect(interface, SIGNAL(Pause()), player, SLOT(Pause()));
  connect(interface, SIGNAL(Stop()), player, SLOT(Stop()));
}

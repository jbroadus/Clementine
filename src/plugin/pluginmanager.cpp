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

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

#include "core/application.h"
#include "core/logging.h"
#include "core/utilities.h"
#include "interfaces/service.h"
#include "plugin.h"

PluginManager::PluginManager(Application* app, QObject* parent)
    : QObject(parent), app_(app) {
  InitPlugins();
}

PluginManager::~PluginManager() {}

void PluginManager::StartAll() {
  for (Plugin* plugin : plugins_) {
    plugin->service_->Start();
  }
}

void PluginManager::InitPlugins() {
  FindPlugins(
      QDir(Utilities::GetConfigPath(Utilities::Path_Root)).filePath("plugins"));
  FindPlugins(
      QCoreApplication::applicationDirPath());
}

void PluginManager::FindPlugins(const QString& path) {
  QDir dir(path);
  for (const QFileInfo& info :
       dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
    if (info.isFile() && QLibrary::isLibrary(info.fileName())) {
      Plugin* plugin = new Plugin(this);
      if (plugin->Load(info.absoluteFilePath())) {
        plugins_ << plugin;
        qLog(Info) << "Loaded plugin" << path;
      } else {
        delete plugin;
      }
    }
  }
}

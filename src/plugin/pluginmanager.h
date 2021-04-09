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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QList>
#include <QObject>

class Application;
class QDir;
class QJsonObject;
class QPluginLoader;
class Plugin;

class PluginManager : public QObject {
 public:
  PluginManager(Application* app, QObject* parent = nullptr);
  ~PluginManager();

  void StartAll();

  Application* app() { return app_; }

 private:
  friend class PluginSettingsCategory;
  friend class PluginManagerSettingsPage;
  const QList<Plugin*>& GetPlugins() { return plugins_; }

 private:
  void InitPlugins();
  void FindPlugins(const QString& path);
  bool LoadPlugin(const QString& name);

  QList<Plugin*> plugins_;

  Application* app_;
};

#endif  // PLUGINMANAGER_H

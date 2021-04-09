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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>

class PluginManager;
class QJsonObject;
class QPluginLoader;

namespace IClementine {
  class Service;
  class ComponentInterface;
  class Player;
  class Settings;
}  // namespace IClementine

class Plugin : public QObject {
 public:
  Plugin(PluginManager* manager);
  virtual ~Plugin();

  QString GetName();

  bool Load(const QString& path);
  bool AddInterfaces(const QJsonObject& metaData, QObject* inst);
  void ConnectComponent(IClementine::ComponentInterface* interface);
  void ConnectPlayer(IClementine::Player* interface);
  void ConnectSettings(IClementine::Settings* interface);

 private:
  friend class PluginSettingsCategory;
  IClementine::Settings* GetSettingsInterface();

 private:
  friend class PluginManager;
  IClementine::Service* service_;

  // Null for static plugins.
  QPluginLoader* loader_;

  PluginManager* mgr_;
};

#endif  // PLUGIN_H

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

#ifndef PLUGINBASE_H
#define PLUGINBASE_H

#include <QObject>

#include "interfaces/service.h"

class PluginBase : public QObject, public IClementine::Service {
  Q_OBJECT
  Q_INTERFACES(IClementine::Service)

 public:
  PluginBase() : QObject(nullptr) {}

  bool Start() override { return true; }
  bool Stop() override { return true; }

  virtual const IClementine::ComponentInterfaceList& GetInterfaces() {
    return interfaces_;
  };
  void AddInterface(IClementine::ComponentInterface* interface) {
    interfaces_ << interface;
  }

protected:
  IClementine::ComponentInterfaceList interfaces_;
};

#endif  // PLUGINBASE_H

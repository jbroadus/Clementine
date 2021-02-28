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

#ifndef INTERFACE_SERVICE_H
#define INTERFACE_SERVICE_H

#include <QObject>
#include "component.h"

namespace IClementine {
  class Service {
   public:
    virtual bool Start() = 0;
    virtual bool Stop() = 0;

    virtual const ComponentInterfaceList& GetInterfaces() = 0;
  };
}

Q_DECLARE_INTERFACE(IClementine::Service, "org.clementine-player.Service")

#endif  // INTERFACE_SERVICE_H

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

#ifndef INTERFACE_UPNPPLUGIN_H
#define INTERFACE_UPNPPLUGIN_H

#include <QObject>

class Application;

namespace IClementine {
class UpnpManager;
class UpnpPlugin {
 public:
  virtual UpnpManager* MakeUpnpManager(Application* app) = 0;
};
}  // namespace IClementine

Q_DECLARE_INTERFACE(IClementine::UpnpPlugin, "org.clementine-player.UpnpPlugin")

#endif  // INTERFACE_UPNPPLUGIN_H

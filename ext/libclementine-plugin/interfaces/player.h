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

#ifndef INTERFACE_PLAYER_H
#define INTERFACE_PLAYER_H

#include "component.h"

namespace IClementine {
  class Player : public ComponentInterface {
   Q_OBJECT
  public:
    Player(QObject* parent) : ComponentInterface(parent) {}

    const QString GetName() override { return "Player"; }
   signals:
    void Play();
    void Pause();
    void Stop();

   public slots:
    virtual void Playing() {};
    virtual void Paused() {};
    virtual void Stopped() {};
  };
};

#endif  // INTERFACE_PLAYER_H

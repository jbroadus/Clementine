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

#ifndef UPNPACTION_H
#define UPNPACTION_H

#include <QString>
#include <QVector>

class UpnpManager;

namespace Clementine {
class UpnpActionRequest;
};

class UpnpAction {
 public:
  UpnpAction() : action_cb_(nullptr) {}
  UpnpAction(const QString& name, bool defaults = true);

  class Arg {
   public:
    enum Dir { IN, OUT };

    Arg() {}
    Arg(const QString& name, const QString& var, Dir dir)
        : name_(name), var_(var), dir_(dir) {}

    const QString dir() const {
      switch (dir_) {
        case IN:
          return "in";
        case OUT:
          return "out";
      }
      return "unknown";
    }

    QString name_;
    QString var_;
    Dir dir_;
  };
  typedef QVector<Arg> ArgList;

  void arg(const QString& name, const QString& varName, Arg::Dir dir);

  ArgList args_;
  QString name_;

  std::function<bool(Clementine::UpnpActionRequest* request)> action_cb_;
};

#endif  // UPNPACTION_H

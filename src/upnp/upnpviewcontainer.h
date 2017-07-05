/* This file is part of Clementine.
   Copyright 2017, Jim Broadus <jbroadus@gmail.com>

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

#ifndef UPNPVIEWCONTAINER_H
#define UPNPVIEWCONTAINER_H

#include <QWidget>

namespace Ui {
class UpnpViewContainer;
}

class UpnpView;

class UpnpViewContainer : public QWidget {
  Q_OBJECT

 public:
  explicit UpnpViewContainer(QWidget* parent = nullptr);
  ~UpnpViewContainer();

  UpnpView* view() const;

 protected:
  void showEvent(QShowEvent*);

 private:
  Ui::UpnpViewContainer* ui_;
  bool loaded_icons_;
};

#endif  // UPNPVIEWCONTAINER_H

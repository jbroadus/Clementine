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

#include "upnpviewcontainer.h"
#include "ui_upnpviewcontainer.h"
#include "ui/iconloader.h"

UpnpViewContainer::UpnpViewContainer(QWidget* parent)
    : QWidget(parent), ui_(new Ui::UpnpViewContainer), loaded_icons_(false) {
  ui_->setupUi(this);
}

UpnpViewContainer::~UpnpViewContainer() { delete ui_; }

void UpnpViewContainer::showEvent(QShowEvent* e) {
  if (!loaded_icons_) {
    loaded_icons_ = true;
    /*
    ui_->close_frame_button->setIcon(IconLoader::Load("edit-delete", 
                                                      IconLoader::Base));
    ui_->warning_icon->setPixmap(IconLoader::Load("dialog-warning", 
                                                  IconLoader::Base).pixmap(22));
    */
  }

  QWidget::showEvent(e);
}

UpnpView* UpnpViewContainer::view() const { return ui_->view; }

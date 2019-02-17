/* This file is part of Clementine.
   Copyright 2019, Jim Broadus <jbroadus@gmail.com>

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

#include <memory>

#include "core/logging.h"

#include "connecteddevice.h"
#include "devicebrowser.h"
#include "devicemanager.h"
#include "ui_devicebrowser.h"

DeviceBrowser::DeviceBrowser(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui_DeviceBrowser) {
  ui_->setupUi(this);
}

DeviceBrowser::~DeviceBrowser() {}

void DeviceBrowser::SetDeviceManager(DeviceManager* manager) {
  manager_ = manager;
}

void DeviceBrowser::BrowseDevice(QModelIndex idx) {
  device_ = manager_->GetConnectedDevice(idx);
  if (!device_) {
    qLog(Error) << "Device does not exist";
    return;
  }
  QAbstractItemModel* browse_model = device_->browse_model();
  if (!browse_model) {
    qLog(Warning) << "Device does not support browsing";
    return;
  }
  ui_->device_tree->setModel(browse_model);
  ui_->device_tree->setRootIndex(device_->browse_root());
  /* Only show the directory name column. */
  for (int i=1; i<browse_model->columnCount(); i++)
    ui_->device_tree->hideColumn(i);
  show();
}

void DeviceBrowser::accept() {
  QDialog::accept();
  QModelIndex index = ui_->device_tree->currentIndex();
  device_->set_browse_root(index);
  device_.reset();
}

void DeviceBrowser::reject() {
  QDialog::reject();
  device_.reset();
}

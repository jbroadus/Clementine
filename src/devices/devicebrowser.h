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

#ifndef DEVICES_DEVICEBROWSER_H_
#define DEVICES_DEVICEBROWSER_H_

#include <memory>

#include <QDialog>
#include <QPersistentModelIndex>

class ConnectedDevice;
class DeviceManager;
class Ui_DeviceBrowser;

class DeviceBrowser : public QDialog {
  Q_OBJECT

 public:
  DeviceBrowser(QWidget* parent = nullptr);
  ~DeviceBrowser();

 public slots:
  void accept();
  void reject();

  void SetDeviceManager(DeviceManager* manager);
  void BrowseDevice(QModelIndex idx);

 private:
  Ui_DeviceBrowser* ui_;

  DeviceManager* manager_;
  std::shared_ptr<ConnectedDevice> device_;
};

#endif  // DEVICES_DEVICEBROWSER_H_

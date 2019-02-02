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

#include <QtConcurrentRun>

#include "core/application.h"
#include "core/logging.h"

#include "upnpdevice.h"
#include "devicemanager.h"
#include "upnpclient.h"
#include "upnpdeviceinfo.h"
#include "upnpserviceinfo.h"
 
UpnpDevice::UpnpDevice(const QUrl& url, DeviceLister* lister,
                       const QString& unique_id, DeviceManager* manager,
                       Application* app, int database_id, bool first_time)
    : ConnectedDevice(url, lister, unique_id, manager, app, database_id,
                      first_time),
      client_(NULL),
      directory_service_(NULL),
      browse_action_(NULL)
{
}

UpnpDevice::~UpnpDevice()
{
}

void UpnpDevice::Init()
{
  client_ = manager_->GetUpnpClient();
}

void UpnpDevice::ConnectAsync()
{
  device_info_ = client_->GetDevice(url_);
  if (!device_info_) {
    app_->AddError(tr("Device no longer exists"));
    emit ConnectFinished(unique_id_, false);
    return;
  }
  QString id("urn:upnp-org:serviceId:ContentDirectory");
  directory_service_ = device_info_->FindServiceById(id);
  if (!directory_service_) {
    app_->AddError(tr("Device did not present a directory service"));
    emit ConnectFinished(unique_id_, false);
    return;
  }

  QtConcurrent::run(this, &UpnpDevice::Connect);
}

bool UpnpDevice::GetBrowseAction()
{
  if (!directory_service_->DownloadSpcd()) {
    app_->AddError(tr("Could not download directory service descriptor"));
    return false;
  }

  QString action_name("Browse");
  browse_action_ = directory_service_->FindActionByName(action_name);
  if (!browse_action_) {
    app_->AddError(tr("No browse action in directory service"));
    return false;
  }
  return true;
}

void UpnpDevice::Browse()
{
  browse_action_->SetInputArgVal("ObjectID", "0");
  browse_action_->SetInputArgVal("BrowseFlag", "BrowseDirectChildren");
  browse_action_->SetInputArgVal("Filter", "*");
  browse_action_->SetInputArgVal("StartingIndex", "0");
  browse_action_->SetInputArgVal("RequestedCount", "10");
  browse_action_->SetInputArgVal("SortCriteria", "");
  
  if (!client_->SendAction(browse_action_, directory_service_)) {
    app_->AddError(tr("Browse action failed"));
    return;
  }

  QString result;
  browse_action_->GetOutputArgVal("Result", result);
  qLog(Info) << result;
}

void UpnpDevice::Connect()
{
  if (GetBrowseAction()) {
    emit ConnectFinished(unique_id_, true);
    QtConcurrent::run(this, &UpnpDevice::Browse);
  } else {
    emit ConnectFinished(unique_id_, false);
  }
}

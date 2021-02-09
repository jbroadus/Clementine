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

#include "upnpserver.h"

#include "upnpmanager.h"
#include "core/logging.h"
#include "core/utilities.h"

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

UpnpServer::UpnpServer(UpnpManager* manager)
  : QObject(manager),
    manager_(manager) {}

UpnpServer::~UpnpServer() {}

bool UpnpServer::Start() {

  base_url_.setScheme("http");
  base_url_.setHost(UpnpGetServerIpAddress());
  base_url_.setPort(UpnpGetServerPort());

  QDir conf_root(GetConfigPath(Utilities::Path_Root));
  root_ = QDir(conf_root.filePath("upnp"));
  if (root_.exists()) {
    qLog(Debug) << "Removing old root";
    root_.removeRecursively();
  }
  if (!conf_root.mkdir("upnp")) {
    qLog(Error) << "Could not create http root directory.";
      return false;
  }
  qLog(Info) << "Using root" << root_.path();

  int ret = UpnpSetWebServerRootDir(root_.absolutePath().toUtf8().constData());

  ret = UpnpEnableWebserver(1);
  if (ret != UPNP_E_SUCCESS) {
    qLog(Error) << "UpnpEnableWebServer error" << UpnpGetErrorMessage(ret);
    return false;
  }

  return true;
}

QString UpnpServer::MakePath(const QString& path) {
  // The Qt file and directory handling APIs are a bit clunky.
  QString relPath = QDir::cleanPath("." + QDir::separator() + path);
  QString absPath = root_.absoluteFilePath(relPath);

  // Create parent directories
  if (!root_.mkpath(QFileInfo(relPath).path())) {
    qLog(Debug) << "Could not create path for" << relPath;
    return QString();
  }
  return absPath;
}

bool UpnpServer::AddFile(const QString& path, const QByteArray& data) {
  QString absPath = MakePath(path);
  if (absPath.isEmpty()) {
    return false;
  }
  
  QFile file(absPath);
  if (!file.open(QIODevice::WriteOnly)) {
    qLog(Debug) << "Could not create" << absPath;
    return false;
  }
  file.write(data);
  return true;
}

bool UpnpServer::AddFile(const QString& path, const QString& src) {
  QString absPath = MakePath(path);
  if (absPath.isEmpty()) {
    return false;
  }

  return QFile::copy(src, absPath);
}

QString UpnpServer::GetUrl(const QString& path) {
  QUrl url = base_url_;
  url.setPath(path);
  return url.toString(QUrl::FullyEncoded);
}

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

#ifndef UPNPSERVER_H
#define UPNPSERVER_H

#include <upnp/upnp.h>

#include <QDir>
#include <QObject>
#include <QUrl>

class UpnpManager;

class UpnpServer : public QObject {
  Q_OBJECT

 public:
  UpnpServer(UpnpManager* manager);
  ~UpnpServer();

  bool Start();
  bool AddFile(const QString& path, const QByteArray& data);
  bool AddFile(const QString& path, const QString& src);

  QString GetUrl(const QString& path);

 private:
  QString MakePath(const QString& path);

  static int SGetInfoCallback(const char* filename, UpnpFileInfo* info,
                              const void* cookie, const void** request_cookie);

  static UpnpWebFileHandle SOpenCallback(const char* filename,
                                         enum UpnpOpenFileMode Mode,
                                         const void* cookie,
                                         const void* request_cookie);

  UpnpManager* manager_;
  QDir root_;
  QUrl base_url_;
};

#endif  // UPNPSERVER_H

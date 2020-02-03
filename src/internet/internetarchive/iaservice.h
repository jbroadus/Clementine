/* This file is part of Clementine.
   Copyright 2020, Jim Broadus  <jbroadus@gmail.com>

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

#ifndef INTERNET_INTERNETARCHIVE_IASERVICE_H_
#define INTERNET_INTERNETARCHIVE_IASERVICE_H_

#include "internet/core/internetservice.h"

class QNetworkReply;
class QXmlStreamReader;

class NetworkAccessManager;

class InternetArchiveQuery;

class InternetArchiveService : public InternetService {
  Q_OBJECT

 public:
  InternetArchiveService(Application* app, InternetModel* parent);
  ~InternetArchiveService();

  static const char* kServiceName;
  static const char* kSettingsGroup;

  QStandardItem* CreateRootItem();
  void LazyPopulate(QStandardItem* item);

 private slots:
  void QueryComplete(InternetArchiveQuery* query, int task_id);
 private:
  void QueryCollection(const QString& name);
  QStandardItem* root_;
  NetworkAccessManager* network_;
};

#endif  // INTERNET_INTERNETARCHIVE_IASERVICE_H_

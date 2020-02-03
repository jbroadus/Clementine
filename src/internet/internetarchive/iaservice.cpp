/* This file is part of Clementine.
   Copyright 2020, Jim Broadus <jbroadus@gmail.com>

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

#include <QNetworkReply>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QtConcurrentRun>

#include "core/application.h"
#include "core/network.h"
#include "core/taskmanager.h"
#include "internet/core/internetmodel.h"
#include "internet/internetarchive/iaservice.h"
#include "internet/internetarchive/iaquery.h"
#include "ui/iconloader.h"

const char* InternetArchiveService::kServiceName = "Internet Archive";
const char* InternetArchiveService::kSettingsGroup = "InternetArchive";

InternetArchiveService::InternetArchiveService(Application* app,
                                               InternetModel* parent)
  : InternetService(kServiceName, app, parent, parent),
    network_(new NetworkAccessManager(this))
{
}

InternetArchiveService::~InternetArchiveService() {
}

QStandardItem* InternetArchiveService::CreateRootItem() {
  root_ = new QStandardItem(IconLoader::Load("icon_radio", IconLoader::Lastfm),
                            kServiceName);
  root_->setData(true, InternetModel::Role_CanLazyLoad);
  return root_;
}

void InternetArchiveService::LazyPopulate(QStandardItem* item) {
  switch (item->data(InternetModel::Role_Type).toInt()) {
    case InternetModel::Type_Service:
      QueryCollection("audio");
      break;
  }
}

void InternetArchiveService::QueryCollection(const QString& name) {
  InternetArchiveQuery *query = new InternetArchiveQuery(network_);

  int task_id =
    app_->task_manager()->StartTask(tr("Downloading collecton %1").arg(name));

  auto complete = [=]() { this->QueryComplete(query, task_id); };
  connect(query, &InternetArchiveQuery::QueryComplete, complete);

  auto error = [=] (const QString& err) {
                 this->app_->task_manager()->SetTaskFinished(task_id);
                 this->app_->AddError(tr("Failed to update collection:\n%1")
                                      .arg(err));
                 delete query;
               };
  connect(query, &InternetArchiveQuery::QueryError, error);
  query->LoadCollection("audio");
}

void InternetArchiveService::QueryComplete(InternetArchiveQuery* query,
                                           int task_id) {
  for (QString& s : query->results_)
    qLog(Debug) << s;
  app_->task_manager()->SetTaskFinished(task_id);

  query->deleteLater();
}

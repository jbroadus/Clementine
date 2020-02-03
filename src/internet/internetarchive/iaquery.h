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

#ifndef INTERNET_INTERNETARCHIVE_IAQUERY_H_
#define INTERNET_INTERNETARCHIVE_IAQUERY_H_

#include <QFuture>
#include <QObject>
#include <QStringList>

class QNetworkReply;
class QXmlStreamReader;

class NetworkAccessManager;

class InternetArchiveQuery : public QObject {
  Q_OBJECT

 public:
  InternetArchiveQuery(NetworkAccessManager* network, QObject* parent = NULL);
  ~InternetArchiveQuery();

  void LoadCollection(const QString& collection);

  QStringList results_;
 signals:
  void QueryComplete();
  void QueryError(const QString& err);

 private slots:
  void DownloadFinished(QNetworkReply* reply);
  void ParseCollectionFinished(QFuture<bool> future);

 private:
  QUrl GetCollectionUrl(const QString& collection);
  bool ParseCollection(QIODevice* device);
  QString ParseDoc(QXmlStreamReader* reader) const;
  void RequestCollection(const QUrl& url);

  NetworkAccessManager* network_;
};

#endif  // INTERNET_INTERNETARCHIVE_IAQUERY_H_

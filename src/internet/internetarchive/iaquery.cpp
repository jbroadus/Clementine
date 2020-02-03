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

#include "core/closure.h"
#include "core/logging.h"
#include "core/network.h"
#include "internet/internetarchive/iaquery.h"

InternetArchiveQuery::InternetArchiveQuery(NetworkAccessManager* network,
                                           QObject* parent)
  : QObject(parent),
    network_(network)
{
}

InternetArchiveQuery::~InternetArchiveQuery() {
}

QUrl InternetArchiveQuery::GetCollectionUrl(const QString& collection) {
  // https://archive.org/advancedsearch.php?q=collection%3Aaudio+AND+mediatype%3Acollection&fl%5B%5D=identifier&sort%5B%5D=&sort%5B%5D=&sort%5B%5D=&rows=50&page=1&callback=callback&save=yes&output=xml
  QUrl url("https://archive.org/advancedsearch.php");
  QUrlQuery query;
  query.addQueryItem("q", "collection:" + collection + "+AND+mediatype:collection");
  query.addQueryItem("rows", "50");
  query.addQueryItem("page", "1");
  query.addQueryItem("output", "xml");
  url.setQuery(query.query());
  return url;
}

void InternetArchiveQuery::LoadCollection(const QString& collection) {
  QUrl url = GetCollectionUrl(collection);
  RequestCollection(url);
}

void InternetArchiveQuery::RequestCollection(const QUrl& url) {
  QNetworkRequest req = QNetworkRequest(url);
  req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                   QNetworkRequest::AlwaysNetwork);
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                   QNetworkRequest::NoLessSafeRedirectPolicy);

  qLog(Debug) << "Requesting" << url;
  QNetworkReply* reply = network_->get(req);
  auto finished = [=]() {
                    this->DownloadFinished(reply);
                  };
  connect(reply, &QNetworkReply::finished, finished);
}

void InternetArchiveQuery::DownloadFinished(QNetworkReply* reply) {
  if (reply->error() != QNetworkReply::NoError) {
    emit QueryError(reply->errorString());
    reply->deleteLater();
    return;
  }

  QFuture<bool> future =
      QtConcurrent::run(this, &InternetArchiveQuery::ParseCollection, reply);
  NewClosure(future, this, SLOT(ParseCollectionFinished(QFuture<bool>)),
             future);
}

void InternetArchiveQuery::ParseCollectionFinished(
    QFuture<bool> future) {
  emit QueryComplete();
}

QString InternetArchiveQuery::ParseDoc(QXmlStreamReader* reader) const
{
  QString item;
  while (!reader->atEnd()) {
    switch(reader->readNext()) {
    case QXmlStreamReader::StartElement:
      if (reader->name() == "str") {
        QXmlStreamAttributes attrs = reader->attributes();
        QString val =
          reader->readElementText(QXmlStreamReader::SkipChildElements);
        if (attrs.value("name") == "title") {
          item = val;
        }
      } else {
        reader->skipCurrentElement();
      }
      break;

    case QXmlStreamReader::EndElement:
      return item;
      break;

    case QXmlStreamReader::Invalid:
      qLog(Debug) << "Invalid XML on line" << reader->lineNumber();
      break;

    default:
      break;

    }
  }
  return item;
}

bool InternetArchiveQuery::ParseCollection(
    QIODevice* device) {
  QXmlStreamReader reader(device);

  while (!reader.atEnd()) {
    switch(reader.readNext()) {
    case QXmlStreamReader::StartElement:
      if (reader.name() == "doc") {
        results_ << ParseDoc(&reader);
      }
      break;

    case QXmlStreamReader::Invalid:
      qLog(Debug) << "Invalid XML on line" << reader.lineNumber();
      break;

    default:
      break;
    }
    //qLog(Debug) << "Read" << reader.tokenString();
  }

  if (reader.hasError()) {
    qLog(Error) << "Error" << reader.errorString() << "line" << reader.lineNumber();
  }
  device->deleteLater();

  return true;
}

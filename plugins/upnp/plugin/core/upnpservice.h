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

#ifndef UPNPSERVICE_H
#define UPNPSERVICE_H

#include <QFile>
#include <QHash>
#include <QObject>
#include <QString>
#include <QXmlStreamWriter>

#include "upnpaction.h"

class UpnpManager;

namespace Clementine {
class UpnpActionRequest;
};

class UpnpVar {
 public:
  UpnpVar() : sendEvents_(false) {}
  UpnpVar(const QString& name) : name_(name), sendEvents_(false) {}

  QString name_;
  QString type_;
  QString value_;
  bool sendEvents_;
};
typedef QVector<UpnpVar> UpnpVarList;

typedef QList<UpnpAction> UpnpActionList;
typedef QHash<QString, UpnpAction> UpnpActionMap;

class UpnpService : public QObject {
 public:
  UpnpService(const QString& name, int rev, UpnpManager* manager,
              QObject* parent);

  virtual bool ParseDesc(const QString& fileName);
  virtual bool WriteDesc();

  // Add service intry for root desc.
  virtual bool FillServiceEntry(QXmlStreamWriter* writer);

  const UpnpActionList actions() { return actionMap_.values(); }
  void AddAction(UpnpAction&& action);
  void ActionRequest(Clementine::UpnpActionRequest* req);

  const QString& id() { return id_; }

 protected:
  virtual void HandleActionRequest(Clementine::UpnpActionRequest* req,
                                   const UpnpAction* action);

  virtual const QString ScpdUrl() { return QString("/%1/desc.xml").arg(name_); }
  virtual const QString ControlUrl() { return QString("/%1/ctrl").arg(name_); }
  virtual const QString EventUrl() { return QString("/%1/event").arg(name_); }

  UpnpAction* GetAction(const QString& name);

  const QString url_;
  const QString name_;
  const QString type_;
  const QString id_;
  const int rev_;

  UpnpManager* manager_;

 protected:
  friend class ScpdParser;
  UpnpActionMap actionMap_;
  UpnpVarList vars_;
};

class UpnpServiceHosted : public UpnpService {
 public:
  UpnpServiceHosted(const QString& name, int rev, UpnpManager* manager,
                    QObject* parent);
  bool WriteDesc();

 protected:
  virtual QString SrcPath();
};

#endif  // UPNPSERVICE_H

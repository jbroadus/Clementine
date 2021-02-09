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
#include <QObject>
#include <QString>
#include <QXmlStreamWriter>

class UpnpManager;

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

class UpnpAction {
public:
  UpnpAction() {}
  UpnpAction(const QString& name, bool defaults = true);

  class Arg {
  public:
    enum Dir { IN, OUT };

    Arg() {}
    Arg(const QString& name, const QString& var, Dir dir)
      : name_(name),
        var_(var),
        dir_(dir) {}

    const QString dir() const {
      switch (dir_) {
      case IN: return "in";
      case OUT: return "out";
      }
      return "unknown";
    }

    QString name_;
    QString var_;
    Dir dir_;
  };
  typedef QVector<Arg> ArgList;

  void arg(const QString& name, const QString& varName, Arg::Dir dir);

  ArgList args_;
  QString name_;
};
  
typedef QVector<UpnpAction> UpnpActionList;


class UpnpService : public QObject {
 public:
  UpnpService(const QString& name, int rev, UpnpManager* manager, QObject* parent);

  virtual bool ParseDesc(const QString& fileName);
  virtual bool WriteDesc();

  // Add service intry for root desc.
  virtual bool FillServiceEntry(QXmlStreamWriter* writer);

  const UpnpActionList& actions() { return actions_; }

  const QString& id() { return id_; }
 protected:
  virtual const QString ScpdUrl() { return QString("/%1/desc.xml").arg(name_); }
  virtual const QString ControlUrl() { return QString("/%1/ctrl").arg(name_); }
  virtual const QString EventUrl() { return QString("/%1/event").arg(name_); }
  
  const QString url_;
  const QString name_;
  const QString id_;
  const int rev_;

  UpnpManager* manager_;

 protected:
  friend class ScpdParser;
  UpnpActionList actions_;
  UpnpVarList vars_;
};

class UpnpServiceHosted : public UpnpService {
 public:
  UpnpServiceHosted(const QString& name, int rev, UpnpManager* manager, QObject* parent);
  bool WriteDesc();
 protected:
  virtual QString SrcPath();
};

#endif  // UPNPSERVICE_H

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

#ifndef UPNPSERVICEINFO_H
#define UPNPSERVICEINFO_H

#include <QList>
#include <QString>
#include <QUrl>

class UpnpDescDoc;
class UpnpDescElement;

class UpnpServiceInfo;

class UpnpStateVarInfo {
public:
  bool Populate(UpnpDescElement& element);
  QString name_;
  QString value_;
  typedef enum {TYPE_I4, TYPE_UI4, TYPE_STR} datatype_t;
  datatype_t data_type_;
  bool send_events_;
};

class UpnpActionArgInfo {
public:
  bool Populate(UpnpDescElement& element, UpnpServiceInfo *service);
  QString name_;
  UpnpStateVarInfo *rel_state_var_;
  bool input_;
};

class UpnpActionInfo {
public:
  bool Populate(UpnpDescElement& element);
  bool PopulateArgs(UpnpDescElement& element, UpnpServiceInfo *service);
  bool UpdateFromResponse(UpnpDescDoc& Doc);

  static UpnpActionArgInfo *FindArgByName(QList<UpnpActionArgInfo> &args,
                                          const QString &name);

  UpnpActionArgInfo *AddInputArg(UpnpActionArgInfo& arg);
  UpnpActionArgInfo *AddOutputArg(UpnpActionArgInfo& arg);
  UpnpActionArgInfo *AddArg(UpnpServiceInfo *service,
                            UpnpDescElement &element);

  UpnpActionArgInfo *FindInputArgByName(const QString &name) {
    return FindArgByName(in_args_, name);
  }

  UpnpActionArgInfo *FindOutputArgByName(const QString &name) {
    return FindArgByName(out_args_, name);
  }

  UpnpStateVarInfo *GetRelatedVarForInputArg(const QString &name);
  UpnpStateVarInfo *GetRelatedVarForOutputArg(const QString &name);
  bool SetInputArgVal(const QString& name, const QString& val);
  bool SetOutputArgVal(const QString& name, const QString& val);
  bool GetInputArgVal(const QString& name, QString& val);
  bool GetOutputArgVal(const QString& name, QString& val);

  QString name_;
  QList<UpnpActionArgInfo> in_args_;
  QList<UpnpActionArgInfo> out_args_;
};


class UpnpServiceInfo {
public:
  static const char *tag_serviceType;
  static const char *tag_serviceId;
  static const char *tag_SCPDURL;
  static const char *tag_controlURL;
  static const char *tag_eventSubURL;

  bool Populate(UpnpDescElement& element);

  UpnpActionInfo *AddAction(UpnpActionInfo &action);
  UpnpActionInfo *AddAction(UpnpDescElement &element);
  UpnpStateVarInfo *AddStateVar(UpnpStateVarInfo &var);
  UpnpStateVarInfo *AddStateVar(UpnpDescElement &elem);
  UpnpActionInfo *FindActionByName(QString &name);
  UpnpStateVarInfo *FindStateVarByName(QString &name);

  bool DownloadSpcd();
  bool ParseSpcd(UpnpDescDoc &doc);
  void GetActionsFromSpcd(UpnpDescDoc &doc);
  void GetStateTableFromSpcd(UpnpDescDoc &doc);

  QString type_;
  QString id_;
  QUrl scpd_url_;
  QUrl control_url_;
  QUrl event_sub_url_;
  bool have_scpd_;
  QList<UpnpActionInfo> actions_;
  QList<UpnpStateVarInfo> state_vars_;
};

#endif /* UPNPSERVICEINFO_H */

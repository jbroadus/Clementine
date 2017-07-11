/* This file is part of Clementine.
   Copyright 2017, Jim Broadus <jbroadus@gmail.com>

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

#ifndef UPNPDATATYPES_H
#define UPNPDATATYPES_H

#include <QList>
#include <QString>


struct UpnpStateVarInfo {
  QString name;
  QString value;
  typedef enum {TYPE_I4, TYPE_UI4, TYPE_STR} datatype_t;
  datatype_t dataType;
  bool sendEvents;
};
typedef QList<UpnpStateVarInfo> UpnpStateVarList;


struct UpnpActionArgInfo {
  QString name;
  UpnpStateVarInfo *relStateVar;
};
typedef QList<UpnpActionArgInfo> UpnpActionArgList;


struct UpnpActionInfo {
  QString name;
  UpnpActionArgList in_args;
  UpnpActionArgList out_args;
  typedef enum {
    ID_SetAVTransportURI,
    ID_SetNextAVTransportURI,
    ID_GetMediaInfo,
    ID_GetTransportInfo,
    ID_GetPositionInfo,
    ID_GetDeviceCapabilities,
    ID_GetTransportSettings,
    ID_Stop,
    ID_Play,
    ID_Pause,
    ID_Seek,
    ID_Next,
    ID_Previous,
  } id_t;
  id_t id;
  
  static UpnpActionArgInfo *FindArgByName(UpnpActionArgList &args,
                                                QString &name) {
    UpnpActionArgList::iterator i;
    for (i = args.begin(); i != args.end(); i++) {
      if (i->name == name)
        return &(*i); 
    }
    return NULL;
  }

  UpnpActionArgInfo *FindInArgByName(QString &name) {
    return FindArgByName(in_args, name);
  }

  UpnpActionArgInfo *FindOutArgByName(QString &name) {
    return FindArgByName(out_args, name);
  }

  UpnpActionArgInfo *FindOutArgByName(const char *name) {
    QString n(name);
    return FindArgByName(out_args, n);
  }

  UpnpStateVarInfo *GetRelatedVarForInArg(const char *name)
  {
    QString n(name);
    UpnpActionArgInfo *arg = FindInArgByName(n);
    if (arg == NULL)
      return NULL;
    return arg->relStateVar;
  }
  
  bool SetOutArgVal(const char *name, const char *val)
  {
    UpnpActionArgInfo *arg = FindOutArgByName(name);
    if (arg == NULL || arg->relStateVar == NULL)
      return false;
    arg->relStateVar->value = val;
    return true;
  }

  
};
typedef QList<UpnpActionInfo> UpnpActionList;


struct UpnpServiceInfo {
  QString type;
  QString id;
  QString scpdUrl;
  QString controlUrl;
  QString eventSubUrl;
  UpnpActionList actions;
  UpnpStateVarList stateVars;
  UpnpActionInfo *FindActionByName(QString &name) {
    UpnpActionList::iterator i;
    for (i = actions.begin(); i != actions.end(); i++) {
      if (i->name == name)
        return &(*i); 
    }
    return NULL;
  }
};
typedef QList<UpnpServiceInfo> UpnpServiceList;


struct UpnpDeviceInfo {
  QString udn;
  QString name;
  QString type;
  UpnpServiceList services;
  UpnpServiceInfo *FindServiceById(QString &id) {
    UpnpServiceList::iterator i;
    for (i = services.begin(); i != services.end(); i++) {
      if (i->id == id)
        return &(*i); 
    }
    return NULL;
  }
};

#endif /* UPNPDATATYPES_H */

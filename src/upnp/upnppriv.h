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

#ifndef UPNPPRIV_H
#define UPNPPRIV_H

#include <QObject>

#include "upnpmanager.h"

#include <upnp/upnp.h>

struct UpnpDeviceInfo;

class UpnpManagerPriv : public QObject {
  Q_OBJECT

public:
  UpnpManagerPriv();
  ~UpnpManagerPriv() {}
  void SetMgr(UpnpManager *mgr);

signals:
  void AddDevice(UpnpDeviceInfo *info);
  void DoAction(UpnpActionInfo *action);

protected:
  static int ClientEventCallback(Upnp_EventType EventType, void *Event, void *Cookie);
  static int RendererEventCallback(Upnp_EventType EventType, void *Event, void *Cookie);

  bool StartAsyncSearch();
private:
  UpnpClient_Handle clientHandle;
  UpnpDevice_Handle rendererHandle;
  UpnpManager *mgr_;
  QString webdir_;

  UpnpDeviceInfo renderer_info_;
  
  /* Event Callbacks */
  int DiscoveryCallback(Upnp_EventType EventType,
                        struct Upnp_Discovery *discovery);

  int ActionReqCallback(struct Upnp_Action_Request *request);


  /* Local device helpers */
  UpnpServiceInfo *AddService(UpnpDeviceInfo &info, UpnpServiceInfo &service);
  UpnpServiceInfo *AddService(UpnpDeviceInfo &info, const char *name);
  UpnpServiceInfo *AddService(UpnpDeviceInfo &info, IXML_Element *element);
  UpnpActionInfo *AddAction(UpnpServiceInfo *info, const char *name,
                            UpnpActionInfo::id_t id);
  UpnpActionArgInfo *AddActionArg(UpnpActionInfo *info, const char *name,
                                  UpnpStateVarInfo *related, bool input);
  UpnpStateVarInfo *AddStateVar(UpnpServiceInfo *info, const char *name,
                                bool sendEvents,
                                UpnpStateVarInfo::datatype_t type);
  bool AddRenderingControlService(UpnpDeviceInfo &info);
  bool AddConnectionManagerService(UpnpDeviceInfo &info);
  bool AddAvTransportService(UpnpDeviceInfo &info);
  bool BuildRendererInfo(UpnpDeviceInfo &info);

  bool CreateClient();
  bool CreateRenderer();

  /* XML parsing helpers */
  void ParseDoc(IXML_Document *doc);
  void GetServices(IXML_Document *doc, UpnpDeviceInfo *devInfo);
  int GetNodeStr(QString &str, IXML_Document *doc, const char *name);
  int GetNodeStr(QString &str, IXML_Element *doc, const char *name);

};

#endif  // UPNPPRIV_H

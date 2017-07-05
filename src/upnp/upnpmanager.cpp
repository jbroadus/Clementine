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

#include "upnpmanager.h"
#include "core/logging.h"

#include <upnp/upnp.h>



class UpnpManagerPriv {
public:
  UpnpManagerPriv();
  void SetMgr(UpnpManager *mgr);

protected:
  static int EventCallback(Upnp_EventType EventType, void *Event, void *Cookie);

private:
  UpnpClient_Handle myHandle;
  UpnpManager *mgr_;

  int DiscoveryCallback(Upnp_EventType EventType,
                        struct Upnp_Discovery *discovery);
  void ParseDoc(IXML_Document *doc);
  int GetNodeStr(QString &str, IXML_Document *doc, const char *name);
};

UpnpManagerPriv::UpnpManagerPriv() :
  myHandle (-1),
  mgr_ (NULL)
{
  int rc;
  char *addr = NULL;
  unsigned short port = 0;

  rc = UpnpInit(addr, port);
  if (rc != UPNP_E_SUCCESS) {
    return;
  }

  addr = UpnpGetServerIpAddress();
  port = UpnpGetServerPort();
  qLog(Debug) << "UPnP on " << addr << ":" << port;

}

void UpnpManagerPriv::SetMgr(UpnpManager *mgr)
{
  int rc;

  mgr_ = mgr;

  rc = UpnpRegisterClient(EventCallback, this, &myHandle);
  if (rc != UPNP_E_SUCCESS) {
    return;
  }

  //UpnpSearchAsync(myHandle, 30, "ssdp:all", NULL);
  UpnpSearchAsync(myHandle, 30, "urn:schemas-upnp-org:device:MediaServer:1", this);
}

int UpnpManagerPriv::GetNodeStr(QString &str, IXML_Document *doc, const char *name)
{
  IXML_NodeList *nl;
  IXML_Node *n, *tn;
  nl = ixmlDocument_getElementsByTagName(doc, name);
  if (nl) {
    n = ixmlNodeList_item(nl, 0);
    tn = ixmlNode_getFirstChild(n);
    str = ixmlNode_getNodeValue(tn);
    ixmlNodeList_free(nl);
    return 0;
  }
  else {
    qLog(Error) << "Could not find tag " << name;
    return -1;
  }
}

void UpnpManagerPriv::ParseDoc(IXML_Document *doc)
{
  QString udn;
  if (GetNodeStr(udn, doc, "UDN") != 0) {
    return;
  }

  if (mgr_->FindDeviceByUdn(udn) == -1) {
    UpnpManager::UpnpDevice dev;
    dev.udn = udn;
    GetNodeStr(dev.name, doc, "friendlyName");
    GetNodeStr(dev.type, doc, "deviceType");
    mgr_->AddDevice(dev);
  }
}

int UpnpManagerPriv::DiscoveryCallback(Upnp_EventType EventType,
                                       struct Upnp_Discovery *discovery)
{
  IXML_Document *DescDoc = NULL;
  if (discovery->ErrCode != UPNP_E_SUCCESS) {
    qLog(Error) << "UPnP discovery error: " << discovery->ErrCode;
    return 0;
  }

  qLog(Debug) << discovery->Location;
  UpnpDownloadXmlDoc(discovery->Location, &DescDoc);
  ParseDoc(DescDoc);

  return 0;
}

int UpnpManagerPriv::EventCallback(Upnp_EventType EventType, void *Event, void *Cookie)
{
  UpnpManagerPriv *priv = (UpnpManagerPriv *)Cookie;
  Q_ASSERT(priv);

  switch (EventType) {
  case UPNP_DISCOVERY_SEARCH_RESULT:
    qLog(Debug) << "UPNP_DISCOVERY_SEARCH_RESULT";
    return priv->DiscoveryCallback(EventType, (struct Upnp_Discovery *)Event);

  case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
#if 0
    qLog(Debug) << "UPNP_DISCOVERY_ADVERTISEMENT_ALIVE";
    return discoveryCallback(EventType, (struct Upnp_Discovery *)Event);
#endif
  default:
    //qLog(Debug) << "EventCallback event " << EventType;
    break;
  }
  return 0;
}



UpnpManager::UpnpManager(Application* app, QObject* parent)
    : QAbstractListModel(parent),
      app_(app)
{
  priv_ = new UpnpManagerPriv();
  priv_->SetMgr(this);
}

UpnpManager::~UpnpManager()
{
  delete priv_;
}

int UpnpManager::FindDeviceByUdn(const QString& udn) const {
  for (int i = 0; i < devices_.count(); ++i) {
    if (devices_[i].udn == udn) return i;
  }
  return -1;
}

void UpnpManager::AddDevice(UpnpDevice &dev)
{
    qLog(Debug) << "New device";
    qLog(Debug) << dev.udn;
    qLog(Debug) << dev.name;
    qLog(Debug) << dev.type;
    beginInsertRows(QModelIndex(), devices_.count(), devices_.count());
    devices_ << dev;
    endInsertRows();
}

int UpnpManager::rowCount(const QModelIndex&) const {
  return devices_.count();
}

QVariant UpnpManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0) return QVariant();

  const UpnpDevice& dev = devices_[index.row()];
    switch (role) {
    case Qt::DisplayRole: {
      QString text;
      if (!dev.name.isEmpty())
        text = dev.name;
      else
        text = dev.udn;

      return text;
    }
    default:
      return QVariant();
    }
}

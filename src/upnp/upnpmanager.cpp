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

#include <QDomDocument>

#include "upnppriv.h"
#include "core/application.h"
#include "core/logging.h"
#include "core/player.h"
#include "playlist/songmimedata.h"



UpnpManager::UpnpManager(Application* app, QObject* parent)
  : SimpleTreeModel<UpnpItem>(new UpnpItem(this), parent),
  app_(app)
{
  priv_ = new UpnpManagerPriv();

  /* Thse signals will come from a non QT thread. */
  if (!connect(priv_, SIGNAL(AddDevice(UpnpDeviceInfo *)),
               SLOT(AddDevice(UpnpDeviceInfo *)),
               Qt::BlockingQueuedConnection))
    qLog(Error) << "AddDevice Connect failed";

  if (!connect(priv_, SIGNAL(DoAction(UpnpActionInfo *)),
               SLOT(DoAction(UpnpActionInfo *)),
               Qt::BlockingQueuedConnection))
      qLog(Error) << "DoAction connect failed";

  connect(this, SIGNAL(Pause()), app_->player(), SLOT(Pause()));
  connect(this, SIGNAL(Play()), app_->player(), SLOT(PlayPause()));
  connect(this, SIGNAL(Stop()), app_->player(), SLOT(Stop()));

  priv_->SetMgr(this);
}

UpnpManager::~UpnpManager()
{
  delete priv_;
}

int UpnpManager::FindDeviceByUdn(const QString& udn) const {
  for (int i = 0; i < root_->children.count(); ++i) {
    UpnpDevice *dev = static_cast<UpnpDevice *>(root_->children[i]);
    Q_ASSERT(dev);
    if (dev->info_->udn == udn) return i;
  }
  return -1;
}

void UpnpManager::LazyPopulate(UpnpItem *item)
{
  if (item->type == UpnpItem::Upnp_Root) {
    
  }
  return;
}

SongMimeData *UpnpManager::MetaToMimeData(QString &meta, QString &uri)
{
  SongMimeData *data = new SongMimeData();

  Song song;

  QDomDocument doc;
  QDomElement elem;
  doc.setContent(meta);
  QDomNodeList items=doc.elementsByTagName("item");
  if (!items.isEmpty()) {
    QDomNode item = items.item(0);
    QDomElement title = item.firstChildElement("dc:title");
    QDomElement artist = item.firstChildElement("upnp:artist");
    QDomElement album = item.firstChildElement("upnp:album");
    QDomElement res = item.firstChildElement("res");
    song.Init(title.text(), artist.text(), album.text(), 0);
  }
  song.set_url(QUrl(uri));

  data->songs << song;
  return data;
}

void UpnpManager::DoAction(UpnpActionInfo *action)
{
  switch(action->id) {

  case UpnpActionInfo::ID_GetTransportInfo:
    //qLog(Debug) << "Action GetTransportInfo";
    if (!action->SetOutArgVal("CurrentTransportState","STOPPED"))
      qLog(Error) << "FAIL";

    action->SetOutArgVal("CurrentSpeed","1");

    switch(app_->player()->GetState()) {
    case Engine::Empty:
      action->SetOutArgVal("CurrentTransportStatus","OK");
      action->SetOutArgVal("CurrentTransportState", "NO_MEDIA_PRESENT");
      break;
    case Engine::Idle:
      action->SetOutArgVal("CurrentTransportStatus","OK");
      action->SetOutArgVal("CurrentTransportState", "STOPPED");
      break;
    case Engine::Playing:
      action->SetOutArgVal("CurrentTransportStatus","OK");
      action->SetOutArgVal("CurrentTransportState", "PLAYING");
      break;
    case Engine::Paused:
      action->SetOutArgVal("CurrentTransportStatus","OK");
      action->SetOutArgVal("CurrentTransportState", "PAUSED_PLAYBACK");
      break;
    case Engine::Error:
      action->SetOutArgVal("CurrentTransportStatus","ERROR_OCCURRED");
      action->SetOutArgVal("CurrentTransportState", "STOPPED");
      break;
    }
    break;

  case UpnpActionInfo::ID_Pause:
    //qLog(Debug) << "PAUSE";
    emit Pause();
    break;

  case UpnpActionInfo::ID_Play:
    //qLog(Debug) << "PLAY";
    emit Play();
    break;

  case UpnpActionInfo::ID_SetAVTransportURI:
    {
      UpnpStateVarInfo *uri = action->GetRelatedVarForInArg("CurrentURI");
      UpnpStateVarInfo *meta = action->GetRelatedVarForInArg("CurrentURIMetaData");
      if (uri == NULL || meta == NULL)
        return;

      //qLog(Debug) << "URI " << uri->value;

      SongMimeData *data = MetaToMimeData(meta->value, uri->value);

      emit AddToPlaylist(data);
    }
    break;

  case UpnpActionInfo::ID_Stop:
    //qLog(Debug) << "STOP";
    emit Stop();
    break;

  default:
    qLog(Debug) << "Unhanded action " << action->name;
  }
}

void UpnpManager::AddDevice(UpnpDeviceInfo *info)
{
  /* This is already checked on the other side of the signal, but due to the
     async nature of libupnp, there may be multiple signals in the pipeline. */
  int idx = FindDeviceByUdn(info->udn);

  if (idx != -1) {
    delete info;
    return;
  }

  qLog(Debug) << "New device";
  qLog(Debug) << info->udn;
  qLog(Debug) << info->name;
  qLog(Debug) << info->type;

  beginInsertRows(ItemToIndex(root_), root_->children.count(),
                    root_->children.count());
  UpnpDevice *dev = new UpnpDevice(info, root_);
  endInsertRows();

  beginInsertRows(ItemToIndex(dev->servicesItem_),
                  dev->servicesItem_->children.count(),
                  dev->servicesItem_->children.count());
  for (int i=0; i<dev->info_->services.count(); i++) {
    AddServiceItem(dev->info_->services[i], dev);
  }
  endInsertRows();
}

void UpnpManager::AddServiceItem(UpnpServiceInfo &info, UpnpDevice *dev)
{
  qLog(Debug) << "New service";
  qLog(Debug) << info.type;

  new UpnpService(info, dev->servicesItem_);
}

QVariant UpnpManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0) return QVariant();

  const UpnpItem *item = IndexToItem(index);

  switch (role) {
  case Qt::DisplayRole: {
    return item->display_text;
  }
  default:
    return QVariant();
  }
}

Qt::ItemFlags UpnpManager::flags(const QModelIndex& index) const {
  const UpnpItem *item = IndexToItem(index);
  switch (item->type) {
  case UpnpItem::Upnp_Service:
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  default:
    return Qt::ItemIsEnabled;
  }
}

/* This file is part of Clementine.
   Copyright 2017, Jim Broadus <jbroadus@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULclass Application;
AR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "upnpview.h"

#include <QMouseEvent>
#include <QPainter>
#include <QSortFilterProxyModel>

#include "upnpmanager.h"

#include <core/application.h>
#include "core/logging.h"

UpnpView::UpnpView(QWidget* parent)
  : AutoExpandingTreeView(parent),
    app_(nullptr),
    sort_model_(nullptr)
{
}

UpnpView::~UpnpView()
{
  delete sort_model_;
}

void UpnpView::SetApplication(Application* app) {
  Q_ASSERT(app_ == nullptr);
  app_ = app;

  connect(app_->upnp_manager(), SIGNAL(DeviceDiscovered(int)),
          SLOT(DeviceDiscovered(int)));

  sort_model_ = new QSortFilterProxyModel(this);
  sort_model_->setSourceModel(app_->upnp_manager());
  sort_model_->setDynamicSortFilter(true);
  sort_model_->setSortCaseSensitivity(Qt::CaseInsensitive);
  sort_model_->sort(0);

  connect(sort_model_,
          SIGNAL(SubModelReset(QModelIndex, QAbstractItemModel*)),
          SLOT(RecursivelyExpand(QModelIndex)));

  setModel(sort_model_);
  connect(this, SIGNAL(doubleClicked(QModelIndex)),
          SLOT(ItemDoubleClicked(QModelIndex)));
}

void UpnpView::DeviceDiscovered(int row)
{
  qLog(Debug) << "DeviceDiscovered";
}

void UpnpView::ItemDoubleClicked(const QModelIndex& index)
{
  qLog(Debug) << "ItemDoubleClicked " << index;
}

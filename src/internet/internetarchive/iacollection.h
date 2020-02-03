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

#ifndef INTERNET_INTERNETARCHIVE_IACOLLECTION_H_
#define INTERNET_INTERNETARCHIVE_IACOLLECTION_H_

#include "core/simpletreeitem.h"

class IACollection : public SimpleTreeItem<IACollection>
{
 public:
  IACollection(const QString& name, SimpleTreeModel<IACollection>* model);
};

#endif  // INTERNET_INTERNETARCHIVE_IACOLLECTION_H_

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

#ifndef ENGINES_PLAYBACKREQUEST_H_
#define ENGINES_PLAYBACKREQUEST_H_

#include <QMap>
#include <QUrl>

class MediaPlaybackRequest {
 public:
  MediaPlaybackRequest(const QUrl& url, qint64 beginning_nanosec = 0,
                       qint64 end_nanosec = 0)
    : url_(url),
      beginning_nanosec_(beginning_nanosec),
      end_nanosec_(end_nanosec) {}

  MediaPlaybackRequest()
    : beginning_nanosec_(0),
      end_nanosec_(0) {}

  QUrl url_;

  qint64 beginning_nanosec_;
  qint64 end_nanosec_;
  typedef QMap<QByteArray, QByteArray> HeaderList;
  HeaderList headers_;
};

#endif  // ENGINES_PLAYBACKREQUEST_H_

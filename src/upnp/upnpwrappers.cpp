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

#include "upnpwrappers.h"
#include "core/logging.h"

/* UpnpDescBase */
bool UpnpDescBase::GetNodeStr(QString &str, const char *tag)
{
  IXML_Node *n, *tn;
  IXML_NodeList *nl = GetElementList(tag);
  if (nl) {
    n = ixmlNodeList_item(nl, 0);
    tn = ixmlNode_getFirstChild(n);
    str = ixmlNode_getNodeValue(tn);
    ixmlNodeList_free(nl);
    //qLog(Debug) << "Found " << tag << " is " << str;
    return true;
  }
  else {
    //qLog(Error) << "Could not find tag " << tag;
    return false;
  }
}

bool UpnpDescBase::GetAbsUrl(QString &str, const char *tag)
{
  if (!GetNodeStr(str, tag))
    return false;

  QUrl res = top_->url_.resolved(QUrl(str));
  str = res.toString();

  return true;
}


/* UpnpDescDoc */
void UpnpDescDoc::Print()
{
  if (!doc_)
    return;

  char *xmlbuff = ixmlPrintNode((IXML_Node *)doc_);
  if (xmlbuff) {
    qLog(Debug) << xmlbuff;
    ixmlFreeDOMString(xmlbuff);
  }
}


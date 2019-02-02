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

#ifndef UPNPDOC_H
#define UPNPDOC_H

#include <QUrl>
#include <QString>
#include <upnp/upnp.h>
#include "core/logging.h"

class UpnpDescDoc;

class UpnpDescBase
{
public:
  UpnpDescBase(UpnpDescDoc *top) :
    top_(top) {}

  bool GetNodeStr(QString &str, const char *tag)
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

  bool GetAbsUrl(QUrl &url, const char *name);

  virtual IXML_NodeList *GetElementList(const char *tag) = 0;

  UpnpDescDoc *top_;
};

class UpnpDescDoc : public UpnpDescBase
{
 public:
  UpnpDescDoc(IXML_Document *doc = NULL) :
    UpnpDescBase(this),
    doc_(doc),
    free_doc_(false)
  {
  }

  ~UpnpDescDoc()
  {
    if (free_doc_)
      ixmlDocument_free(doc_);
  }

  virtual IXML_NodeList *GetElementList(const char *tag)
  {
    return ixmlDocument_getElementsByTagName(doc_, tag);
  }

  bool Download(const QUrl& url)
  {
    if (free_doc_) {
      ixmlDocument_free(doc_);
      free_doc_ = false;
    }

    url_ = url;
    QByteArray url_ba = url_.toEncoded();

    UpnpDownloadXmlDoc(url_ba.data(), &doc_);

    if (doc_ != NULL) {
      free_doc_ = true;
      return true;
    }
    else {
      return false;
    }
  }

  void Print()
  {
    if (!doc_)
      return;

    char *xmlbuff = ixmlPrintNode((IXML_Node *)doc_);
    if (xmlbuff) {
      qLog(Debug) << xmlbuff;
      ixmlFreeDOMString(xmlbuff);
    }
  }

  QUrl url_;
  IXML_Document *doc_;
  bool free_doc_;
};

class UpnpDescElement : public UpnpDescBase
{
public:
  UpnpDescElement(UpnpDescDoc *top, IXML_Element *elem) :
    UpnpDescBase(top),
    elem_(elem) {}

  virtual IXML_NodeList *GetElementList(const char *tag)
  {
    return ixmlElement_getElementsByTagName(elem_, tag);
  }

  IXML_Element *elem_;
};

class UpnpElementList
{
public:
  UpnpElementList(UpnpDescDoc &doc, const char *listTag, const char *itemTag) :
    nodeList(nullptr),
    nodeCount(0)
  {
    IXML_NodeList *listList =
      ixmlDocument_getElementsByTagName(doc.doc_, listTag);
    if (listList) {
      Init(listList, itemTag);
      ixmlNodeList_free(listList);
    }
  }

  UpnpElementList(UpnpDescElement &elem, const char *listTag, const char *itemTag) :
    nodeList(nullptr),
    nodeCount(0)
  {
    IXML_NodeList *listList =
      ixmlElement_getElementsByTagName(elem.elem_, listTag);
    if (listList) {
      Init(listList, itemTag);
      ixmlNodeList_free(listList);
    }
  }

  ~UpnpElementList()
  {
    if (nodeList)
      ixmlNodeList_free(nodeList);
  }

  unsigned int count()
  {
    return nodeCount;
  }

  IXML_Element *get(unsigned int index)
  {
    if (!nodeList || index >= nodeCount)
      return nullptr;
    return (IXML_Element *)ixmlNodeList_item(nodeList, index);
  }

private:
  /* Common constructor bits. */
  void Init(IXML_NodeList *listList, const char *itemTag)
  {
    IXML_Node *tn = ixmlNodeList_item(listList, 0);
    nodeList = ixmlElement_getElementsByTagName((IXML_Element *)tn, itemTag);
    if(nodeList)
      nodeCount = ixmlNodeList_length(nodeList);
  }

  IXML_NodeList *nodeList;
  unsigned int nodeCount;
};

#endif  // UPNPDOC_H

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

#ifndef UPNPWRAPPERS_H
#define UPNPWRAPPERS_H

#include <QUrl>
#include <upnp/upnp.h>

class UpnpDescDoc;

class UpnpDescBase
{
public:
  UpnpDescBase(UpnpDescDoc *top) :
    top_(top) {}

  bool GetNodeStr(QString &str, const char *tag);
  bool GetAbsUrl(QString &str, const char *tag);

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

  bool Download(const char *url)
  {
    if (free_doc_) {
      ixmlDocument_free(doc_);
      free_doc_ = false;
    }

    url_.setUrl(url);

    UpnpDownloadXmlDoc(url, &doc_);

    if (doc_ != NULL) {
      free_doc_ = true;
      return true;
    }
    else {
      return false;
    }
  }

  bool Download(QString url)
  {
    return Download(url.toAscii().data());
  }

  void Print();

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


#endif // UPNPWRAPPERS_H

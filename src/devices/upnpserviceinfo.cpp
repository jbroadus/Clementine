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

#include "upnpserviceinfo.h"
#include "upnpdoc.h"

const char *UpnpServiceInfo::tag_serviceType = "serviceType";
const char *UpnpServiceInfo::tag_serviceId = "serviceId";
const char *UpnpServiceInfo::tag_SCPDURL = "SCPDURL";
const char *UpnpServiceInfo::tag_controlURL = "controlURL";
const char *UpnpServiceInfo::tag_eventSubURL = "eventSubURL";

bool UpnpStateVarInfo::Populate(UpnpDescElement& element)
{
  element.GetNodeStr(name_, "name");
  QString send_event_attr_str;
  element.GetNodeStr(send_event_attr_str, "sendEventsAttribute");
  send_events_ = (send_event_attr_str.compare("yes", Qt::CaseInsensitive) == 0);
  QString data_type_str;
  element.GetNodeStr(data_type_str, "dataType");
  if (data_type_str.compare("ui4", Qt::CaseInsensitive) == 0)
    data_type_ = UpnpStateVarInfo::TYPE_UI4;
  else if (data_type_str.compare("i4", Qt::CaseInsensitive) == 0)
    data_type_ = UpnpStateVarInfo::TYPE_I4;
  else
    data_type_ = UpnpStateVarInfo::TYPE_STR;

  return true;
}

bool UpnpActionArgInfo::Populate(UpnpDescElement& element,
                                 UpnpServiceInfo *service)
{
  element.GetNodeStr(name_, "name");
  QString related_str;
  element.GetNodeStr(related_str, "relatedStateVariable");
  rel_state_var_ = service->FindStateVarByName(related_str);
  QString dir_str;
  element.GetNodeStr(dir_str, "direction");
  input_ = (dir_str.compare("in", Qt::CaseInsensitive) == 0);

  qLog(Debug) << "Arg " << name_ << " " << dir_str;

  return true;
}

bool UpnpActionInfo::Populate(UpnpDescElement& element)
{
  element.GetNodeStr(name_, "name");
  qLog(Debug) << "Action " << name_;
  return true;
}

bool UpnpActionInfo::PopulateArgs(UpnpDescElement& element, UpnpServiceInfo *service)
{
  UpnpElementList list(element, "argumentList", "argument");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement argElem(element.top_, list.get(i));
    AddArg(service, argElem);
  }
  return true;
}

bool UpnpActionInfo::UpdateFromResponse(UpnpDescDoc& doc)
{
  QString val;
  for ( UpnpActionArgInfo& arg : out_args_ ) {
    QByteArray name = arg.name_.toLatin1();
    if (doc.GetNodeStr(val, name.data())) {
      arg.rel_state_var_->value_ = val;
    } else {
      qLog(Warning) << "Did not find " << arg.name_ << " in response";
    }
  }
  return true;
}

UpnpActionArgInfo *UpnpActionInfo::AddInputArg(UpnpActionArgInfo& arg)
{
  in_args_ << arg;
  return &in_args_.last();
}

UpnpActionArgInfo *UpnpActionInfo::AddOutputArg(UpnpActionArgInfo& arg)
{
  out_args_ << arg;
  return &out_args_.last();
}

UpnpActionArgInfo *UpnpActionInfo::AddArg(UpnpServiceInfo *service,
                                          UpnpDescElement& element)
{
  UpnpActionArgInfo arg;
  if (!arg.Populate(element, service))
    return NULL;
  if (arg.input_)
    return AddInputArg(arg);
  else
    return AddOutputArg(arg);
}

UpnpActionArgInfo *UpnpActionInfo::FindArgByName(QList<UpnpActionArgInfo> &args,
    const QString &name)
{
  QList<UpnpActionArgInfo>::iterator i;
  for (i = args.begin(); i != args.end(); i++) {
    if (i->name_ == name)
      return &(*i); 
  }
  return NULL;
}

UpnpStateVarInfo *UpnpActionInfo::GetRelatedVarForInputArg(const QString &name)
{
  UpnpActionArgInfo *arg = FindInputArgByName(name);
  if (arg == NULL)
    return NULL;
  return arg->rel_state_var_;
}

UpnpStateVarInfo *UpnpActionInfo::GetRelatedVarForOutputArg(const QString &name)
{
  UpnpActionArgInfo *arg = FindOutputArgByName(name);
  if (arg == NULL)
    return NULL;
  return arg->rel_state_var_;
}

bool UpnpActionInfo::SetOutputArgVal(const QString& name, const QString& val)
{
  UpnpActionArgInfo *arg = FindOutputArgByName(name);
  if (arg == NULL || arg->rel_state_var_ == NULL)
    return false;
  arg->rel_state_var_->value_ = val;
  return true;
}

bool UpnpActionInfo::SetInputArgVal(const QString& name, const QString& val)
{
  UpnpActionArgInfo *arg = FindInputArgByName(name);
  if (arg == NULL || arg->rel_state_var_ == NULL)
    return false;
  arg->rel_state_var_->value_ = val;
  return true;
}

bool UpnpActionInfo::GetOutputArgVal(const QString& name, QString& val)
{
  UpnpActionArgInfo *arg = FindOutputArgByName(name);
  if (arg == NULL || arg->rel_state_var_ == NULL)
    return false;
  val = arg->rel_state_var_->value_;
  return true;
}

bool UpnpActionInfo::GetInputArgVal(const QString& name, QString& val)
{
  UpnpActionArgInfo *arg = FindInputArgByName(name);
  if (arg == NULL || arg->rel_state_var_ == NULL)
    return false;
  val = arg->rel_state_var_->value_;
  return true;
}

bool UpnpServiceInfo::Populate(UpnpDescElement& element)
{
  element.GetNodeStr(type_, UpnpServiceInfo::tag_serviceType);
  element.GetNodeStr(id_, UpnpServiceInfo::tag_serviceId);
  element.GetAbsUrl(scpd_url_, UpnpServiceInfo::tag_SCPDURL);
  element.GetAbsUrl(event_sub_url_, UpnpServiceInfo::tag_eventSubURL);
  element.GetAbsUrl(control_url_, UpnpServiceInfo::tag_controlURL);
  return true;
}

UpnpActionInfo *UpnpServiceInfo::AddAction(UpnpActionInfo &action)
{
  actions_ << action;
  return &actions_.last();
}

UpnpActionInfo *UpnpServiceInfo::AddAction(UpnpDescElement& elem)
{
  UpnpActionInfo action;
  if (!action.Populate(elem))
    return NULL;
  return AddAction(action);
}

UpnpStateVarInfo *UpnpServiceInfo::AddStateVar(UpnpStateVarInfo &var)
{
  state_vars_ << var;
  return &state_vars_.last();
}

UpnpStateVarInfo *UpnpServiceInfo::AddStateVar(UpnpDescElement& elem)
{
  UpnpStateVarInfo state_var;
  if (!state_var.Populate(elem))
    return NULL;
  return AddStateVar(state_var);
}

UpnpActionInfo *UpnpServiceInfo::FindActionByName(QString &name)
{
  QList<UpnpActionInfo>::iterator i;
  for (i = actions_.begin(); i != actions_.end(); i++) {
    if (i->name_ == name)
      return &(*i); 
  }
  return NULL;
}

UpnpStateVarInfo *UpnpServiceInfo::FindStateVarByName(QString &name)
{
  QList<UpnpStateVarInfo>::iterator i;
  for (i = state_vars_.begin(); i != state_vars_.end(); i++) {
    if (i->name_ == name)
      return &(*i); 
  }
  return NULL;
}

bool UpnpServiceInfo::DownloadSpcd()
{
  UpnpDescDoc doc;
  if (doc.Download(scpd_url_)) {
    return ParseSpcd(doc);
  }
  else {
    qLog(Error) << "Failed to download " << scpd_url_;
    return false;
  }
}

bool UpnpServiceInfo::ParseSpcd(UpnpDescDoc &doc)
{
  GetStateTableFromSpcd(doc);
  GetActionsFromSpcd(doc);
  return true;
}

void UpnpServiceInfo::GetActionsFromSpcd(UpnpDescDoc &doc)
{
  UpnpElementList list(doc, "actionList", "action");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    UpnpActionInfo *action = AddAction(elem);
    action->PopulateArgs(elem, this);
  }
}

void UpnpServiceInfo::GetStateTableFromSpcd(UpnpDescDoc &doc)
{
  UpnpElementList list(doc, "serviceStateTable", "stateVariable");
  for (int i=0; i<list.count(); i++) {
    UpnpDescElement elem(&doc, list.get(i));
    AddStateVar(elem);
  }
}

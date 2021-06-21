/* This file is part of Clementine.
   Copyright 2021, Jim Broadus <jbroadus@gmail.com>

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

#include "scpdparser.h"

#include "core/logging.h"
#include "scpdbuilder.h"
#include "upnpservice.h"

bool ScpdParser::parse() {
  if (!readNextStartElement()) {
    qLog(Error) << "Could not get first element.";
    return false;
  }
  if (!expected("scpd")) {
    return false;
  }

  while (readNextStartElement()) {
    if (isElement("serviceStateTable")) {
      if (!parseServiceStateTable()) {
        return false;
      }
    } else if (isElement("actionList")) {
      if (!parseActionList()) {
        return false;
      }
    } else {
      qLog(Warning) << "Unknown element" << name();
      skipCurrentElement();
    }
  }

  qLog(Debug) << "Scpd parse complete.";

  return true;
}

bool ScpdParser::parseActionList() {
  while (readNextStartElement()) {
    if (!expected("action")) {
      return false;
    }
    if (!parseAction()) {
      return false;
    }
  }
  return true;
}

bool ScpdParser::parseAction() {
  UpnpAction action;
  while (readNextStartElement()) {
    if (isElement("name")) {
      action.name_ = readElementText();
    } else if (isElement("argumentList")) {
      if (!parseArgList(action)) {
        return false;
      }
    } else if (isElement("Optional")) {
      // TODO
      skipCurrentElement();
    } else {
      qLog(Debug) << "Skip" << name();
      skipCurrentElement();
    }
  }
  if (action.name_.isEmpty()) {
    qLog(Error) << "Missing required action name.";
    return false;
  }
  service_->AddAction(std::move(action));

  return true;
}

bool ScpdParser::parseArgList(UpnpAction& action) {
  while (readNextStartElement()) {
    if (!expected("argument")) {
      return false;
    }
    if (!parseArg(action)) {
      return false;
    }
  }

  return true;
}

bool ScpdParser::parseArg(UpnpAction& action) {
  UpnpAction::Arg arg;
  while (readNextStartElement()) {
    if (isElement("name")) {
      arg.name_ = readElementText();
    } else if (isElement("direction")) {
      QString dir = readElementText();
      if (QString::compare(dir, "in", Qt::CaseInsensitive) == 0) {
        arg.dir_ = UpnpAction::Arg::IN;
      } else if (QString::compare(dir, "out", Qt::CaseInsensitive) == 0) {
        arg.dir_ = UpnpAction::Arg::OUT;
      } else {
        qLog(Error) << "Unknown direction" << dir;
        return false;
      }
    } else if (isElement("relatedStateVariable")) {
      arg.var_ = readElementText();
    } else {
      qLog(Debug) << "Skip" << name();
      skipCurrentElement();
    }
  }
  if (arg.name_.isEmpty()) {
    qLog(Error) << "Missing required arg name.";
    return false;
  }
  action.args_ << arg;

  return true;
}

bool ScpdParser::parseServiceStateTable() {
  while (readNextStartElement()) {
    if (!expected("stateVariable")) {
      return false;
    }
    if (!parseStateVariable()) {
      return false;
    }
  }
  return true;
}

bool ScpdParser::parseStateVariable() {
  UpnpVar var;
  while (readNextStartElement()) {
    if (isElement("name")) {
      var.name_ = readElementText();
    } else if (isElement("dataType")) {
      var.type_ = readElementText();
    } else if (isElement("sendEventsAttribute")) {
      QString send = readElementText();
      if (QString::compare(send, "yes", Qt::CaseInsensitive) == 0) {
        var.sendEvents_ = true;
      } else if (QString::compare(send, "no", Qt::CaseInsensitive) == 0) {
        var.sendEvents_ = false;
      } else {
        qLog(Error) << "Unknown value" << send;
        return false;
      }
    } else if (isElement("defaultValue")) {
      var.value_ = readElementText();
    } else if (isElement("allowedValueList")) {
      // TODO
      skipCurrentElement();
    } else if (isElement("allowedValueRange")) {
      // TODO
      skipCurrentElement();
    } else if (isElement("Optional")) {
      // TODO
      skipCurrentElement();
    } else {
      qLog(Debug) << "Skip" << name();
      skipCurrentElement();
    }
  }

  if (var.name_.isEmpty()) {
    qLog(Error) << "Missing required var name.";
    return false;
  }
  service_->AddVar(std::move(var));

  return true;
}

bool ScpdParser::isElement(const QString element) {
  return name().compare(element, Qt::CaseInsensitive) == 0;
}

bool ScpdParser::expected(const QString element) {
  if (isElement(element)) {
    return true;
  } else {
    qLog(Error) << "expected" << element << "element, got" << name();
    return false;
  }
}

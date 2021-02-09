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

#include "scpdbuilder.h"

#include "core/logging.h"
#include "upnpservice.h"

bool ScpdBuilder::build() {
  setAutoFormatting(true);
  QString xmlns = "urn:schemas-upnp-org:service-1-0";
  writeStartDocument();
  writeStartElement("scpd");
  writeAttribute("xmlns", xmlns);

  writeStartElement("specVersion");
  writeTextElement("major", "1");
  writeTextElement("minor", "0");
  writeEndElement();  // specVersion

  writeActionList();

  writeStateTable();

  writeEndElement();  // scpd

  writeEndDocument();

  return true;
}

void ScpdBuilder::writeActionList() {
  writeStartElement("actionList");

  for (const UpnpAction& action : service_->actions()) {
    writeStartElement("action");
    writeTextElement("name", action.name_);

    writeStartElement("argumentList");
    for (const UpnpAction::Arg& arg : action.args_) {
      writeStartElement("argument");
      writeTextElement("name", arg.name_);
      writeTextElement("direction", arg.dir());
      writeTextElement("relatedStateVariable", arg.var_);
      writeEndElement();  // argument
    }
    writeEndElement();  // argumentList
    writeEndElement();  // action
  }

  writeEndElement();  // actionList
}

void ScpdBuilder::writeStateTable() {
  writeStartElement("serviceStateTable");
  writeEndElement();  // serviceStateTable
}

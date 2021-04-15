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

#include "pluginsettingscategory.h"

#include "core/application.h"
#include "core/logging.h"
#include "interfaces/settings.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "pluginmanagersettingspage.h"
#include "pluginsettingspage.h"

PluginSettingsCategory::PluginSettingsCategory(SettingsDialog* dialog)
    : SettingsCategory(SettingsDialog::Page_PluginManager,
                       new PluginManagerSettingsPage(dialog), dialog),
      next_id_(SettingsDialog::Page_PluginManager + 1) {
  AddChildren();
}

void PluginSettingsCategory::AddChildren() {
  for (Plugin* plugin : dialog_->app()->plugin_manager()->GetPlugins()) {
    IClementine::Settings* interface = plugin->GetSettingsInterface();
    if (interface != nullptr) {
      qLog(Debug) << "Add settings for plugin" << plugin->GetName();
      AddPage(SettingsDialog::Page(next_id_++),
              new PluginSettingsPage(interface, dialog_));
    }
  }
}

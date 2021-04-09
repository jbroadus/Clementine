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

#include "pluginmanagersettingspage.h"

#include "core/application.h"
#include "interfaces/service.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "ui_pluginmanagersettingspage.h"

PluginManagerSettingsPage::PluginManagerSettingsPage(SettingsDialog* dialog)
    : SettingsPage(dialog), ui_(new Ui::PluginManagerSettingsPage) {
  ui_->setupUi(this);
}

PluginManagerSettingsPage::~PluginManagerSettingsPage() {}

void PluginManagerSettingsPage::Load() {
  PluginManager* mgr = dialog()->app()->plugin_manager();
  for (Plugin* plugin : mgr->GetPlugins()) {
    ui_->plugin_list->addItem(plugin->GetName());
  }
}

void PluginManagerSettingsPage::Save() {}

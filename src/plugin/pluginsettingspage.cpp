#include "pluginsettingspage.h"

#include "interfaces/settings.h"

PluginSettingsPage::PluginSettingsPage(IClementine::Settings* settings,
                                       SettingsDialog* dialog)
  : SettingsPage(dialog),
    settings_(settings) {
  QWidget* inner = settings->GetSettingsPage();
  inner->setParent(this);
  setWindowTitle(inner->windowTitle());
}

void PluginSettingsPage::Load() {
  settings_->Load();
}

void PluginSettingsPage::Save() {
  settings_->Save();
}

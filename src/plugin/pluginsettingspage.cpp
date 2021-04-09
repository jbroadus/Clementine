#include "pluginsettingspage.h"

PluginSettingsPage::PluginSettingsPage(QWidget* settings,
                                       SettingsDialog* dialog)
    : SettingsPage(dialog) {
  settings->setParent(this);
  setWindowTitle(settings->windowTitle());
}

void PluginSettingsPage::Load() {
}

void PluginSettingsPage::Save() {
}

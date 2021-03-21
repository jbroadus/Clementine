#include "pluginsettingspage.h"

PluginSettingsPage::PluginSettingsPage(QWidget* settings,
                                       SettingsDialog* dialog)
    : SettingsPage(dialog) {
  settings->setParent(this);
}

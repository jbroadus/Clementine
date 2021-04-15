#include "examplesettings.h"

#include "example.h"
#include "examplesettingspage.h"

#include <QSettings>

ExampleSettings::ExampleSettings(QObject* parent)
  : IClementine::Settings(parent),
    page_(new ExampleSettingsPage) {}

QWidget* ExampleSettings::GetSettingsPage() {
  return page_;
}

void ExampleSettings::Load() {
  QSettings s;
  s.beginGroup(ExamplePlugin::kSettingsGroup);
  page_->port.setValue(s.value(ExamplePlugin::kSettingPort,
                               ExamplePlugin::kDefaultPort).toInt());
}

void ExampleSettings::Save() {
  QSettings s;
  s.beginGroup(ExamplePlugin::kSettingsGroup);
  s.setValue(ExamplePlugin::kSettingPort, page_->port->value());
}

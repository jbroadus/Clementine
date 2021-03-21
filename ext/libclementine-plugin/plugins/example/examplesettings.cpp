#include "examplesettings.h"
#include "examplesettingspage.h"

QWidget* ExampleSettings::GetSettingsPage() {
  return new ExampleSettingsPage;
};


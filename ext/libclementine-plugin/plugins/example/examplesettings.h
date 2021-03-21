#ifndef EXAMPLE_SETTINGS_H
#define EXAMPLE_SETTINGS_H

#include "interfaces/settings.h"

class QWidget;

class ExampleSettings : public IClementine::Settings {
  Q_OBJECT
 public:
  ExampleSettings(QObject* parent) : IClementine::Settings(parent) {}

  QWidget* GetSettingsPage();
};

#endif  // EXAMPLE_SETTINGS_H

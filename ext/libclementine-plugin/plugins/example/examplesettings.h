#ifndef EXAMPLE_SETTINGS_H
#define EXAMPLE_SETTINGS_H

#include "interfaces/settings.h"

class ExampleSettingsPage;

class ExampleSettings : public IClementine::Settings {
  Q_OBJECT
 public:
  ExampleSettings(QObject* parent);

  QWidget* GetSettingsPage() override;
  void Load() override;
  void Save() override;

 private:
  ExampleSettingsPage* page_;
};

#endif  // EXAMPLE_SETTINGS_H

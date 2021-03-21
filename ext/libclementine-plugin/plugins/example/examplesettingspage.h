#ifndef EXAMPLESETTINGSPAGE_H
#define EXAMPLESETTINGSPAGE_H

#include <QWidget>
#include <memory>

class Ui_ExampleSettingsPage;

class ExampleSettingsPage : public QWidget {
 public:
  ExampleSettingsPage(QWidget* parent = nullptr);
  ~ExampleSettingsPage();

 private:
  std::unique_ptr<Ui_ExampleSettingsPage> ui_;
};

#endif  // EXAMPLESETTINGSPAGE_H

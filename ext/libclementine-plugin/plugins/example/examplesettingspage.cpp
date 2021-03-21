#include "examplesettingspage.h"
#include "ui_examplesettingspage.h"

ExampleSettingsPage::ExampleSettingsPage(QWidget* parent)
  : QWidget(parent),
    ui_(new Ui::ExampleSettingsPage) {
  ui_->setupUi(this);
}

ExampleSettingsPage::~ExampleSettingsPage() {}

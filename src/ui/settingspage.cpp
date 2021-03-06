/* This file is part of Clementine.
   Copyright 2010, David Sansome <me@davidsansome.com>

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

#include "settingspage.h"

#include "core/logging.h"
#include "settingsdialog.h"

SettingsPage::SettingsPage(SettingsDialog* dialog)
    : QWidget(dialog), maybe_changed_(false), dialog_(dialog) {}

void SettingsPage::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  maybe_changed_ = true;
}

void SettingsPage::Apply() {
  if (maybe_changed_) {
    qLog(Debug) << "Saving" << windowTitle();
    Save();
    if (!isVisible())
      // Don't expect additional changes until the page is visible again.
      maybe_changed_ = false;
  }
}

void SettingsPage::Accept() {
  if (maybe_changed_) {
    qLog(Debug) << "Saving" << windowTitle();
    Save();
    maybe_changed_ = false;
  }
}

void SettingsPage::Reject() {
  Cancel();
  maybe_changed_ = false;
}

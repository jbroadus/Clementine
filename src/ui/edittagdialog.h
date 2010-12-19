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

#ifndef EDITTAGDIALOG_H
#define EDITTAGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QModelIndexList>

#include "core/song.h"
#include "widgets/lineedit.h"

class LibraryBackend;
class Ui_EditTagDialog;

class QItemSelection;

class EditTagDialog : public QDialog {
  Q_OBJECT

public:
  EditTagDialog(QWidget* parent = 0);
  ~EditTagDialog();

  static const char* kHintText;

  bool SetSongs(const SongList& songs);
  void SetTagCompleter(LibraryBackend* backend);

private slots:
  void SelectionChanged();
  void FieldValueEdited();
  void ResetField();

private:
  struct Data {
    Data(const Song& song = Song()) : original_(song), current_(song) {}

    static QVariant value(const Song& song, const QString& id);
    QVariant original_value(const QString& id) const { return value(original_, id); }
    QVariant current_value(const QString& id) const { return value(current_, id); }

    void set_value(const QString& id, const QVariant& value);

    Song original_;
    Song current_;
  };

  struct FieldData {
    FieldData(QLabel* label = NULL, QWidget* editor = NULL,
              const QString& id = QString())
      : label_(label), editor_(editor), id_(id) {}

    QLabel* label_;
    QWidget* editor_;
    QString id_;
  };

  bool DoesValueVary(const QModelIndexList& sel, const QString& id) const;
  bool IsValueModified(const QModelIndexList& sel, const QString& id) const;

  void InitFieldValue(const FieldData& field, const QModelIndexList& sel);
  void UpdateFieldValue(const FieldData& field, const QModelIndexList& sel);
  void ResetFieldValue(const FieldData& field, const QModelIndexList& sel);

private:
  Ui_EditTagDialog* ui_;

  QList<Data> data_;
  QList<FieldData> fields_;

  bool ignore_edits_;
};

#endif // EDITTAGDIALOG_H

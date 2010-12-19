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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpinBox>

#include "ui/iconloader.h"

class QToolButton;

class LineEditInterface {
public:
  LineEditInterface(QWidget* widget) : widget_(widget) {}

  QWidget* widget() const { return widget_; }

  virtual ~LineEditInterface() {}

  virtual void clear() { set_text(QString()); }
  virtual void set_focus() = 0;
  virtual QString text() const = 0;
  virtual void set_text(const QString& text) = 0;

  virtual QString hint() const = 0;
  virtual void set_hint(const QString& hint) = 0;
  virtual void clear_hint() = 0;

protected:
  QWidget* widget_;
};

class ExtendedEditor : public LineEditInterface {
public:
  ExtendedEditor(QWidget* widget, int extra_right_padding = 0, bool draw_hint = true);
  virtual ~ExtendedEditor() {}

  virtual bool is_empty() const { return text().isEmpty(); }

  QString hint() const { return hint_; }
  void set_hint(const QString& hint);
  void clear_hint() { set_hint(QString()); }

  bool has_clear_button() const { return has_clear_button_; }
  void set_clear_button(bool visible);

  bool has_reset_button() const;
  void set_reset_button(bool visible);

protected:
  void Paint(QPaintDevice* device);
  void Resize();

private:
  void UpdateButtonGeometry();

protected:
  QString hint_;

  bool has_clear_button_;
  QToolButton* clear_button_;
  QToolButton* reset_button_;

  int extra_right_padding_;
  bool draw_hint_;
};

class LineEdit : public QLineEdit,
                 public ExtendedEditor {
  Q_OBJECT
  Q_PROPERTY(QString hint READ hint WRITE set_hint);
  Q_PROPERTY(bool has_clear_button READ has_clear_button WRITE set_clear_button);
  Q_PROPERTY(bool has_reset_button READ has_reset_button WRITE set_reset_button);

public:
  LineEdit(QWidget* parent = 0);

  // ExtendedEditor
  void set_focus() { QLineEdit::setFocus(); }
  QString text() const { return QLineEdit::text(); }
  void set_text(const QString& text) { QLineEdit::setText(text); }

protected:
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);

signals:
  void Reset();
};

class TextEdit : public QPlainTextEdit,
                 public ExtendedEditor {
  Q_OBJECT
  Q_PROPERTY(QString hint READ hint WRITE set_hint);
  Q_PROPERTY(bool has_clear_button READ has_clear_button WRITE set_clear_button);
  Q_PROPERTY(bool has_reset_button READ has_reset_button WRITE set_reset_button);

public:
  TextEdit(QWidget* parent = 0);

  // ExtendedEditor
  void set_focus() { QPlainTextEdit::setFocus(); }
  QString text() const { return QPlainTextEdit::toPlainText(); }
  void set_text(const QString& text) { QPlainTextEdit::setPlainText(text); }

protected:
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);

signals:
  void Reset();
};

class SpinBox : public QSpinBox,
                public ExtendedEditor {
  Q_OBJECT
  Q_PROPERTY(QString hint READ hint WRITE set_hint);
  Q_PROPERTY(bool has_clear_button READ has_clear_button WRITE set_clear_button);
  Q_PROPERTY(bool has_reset_button READ has_reset_button WRITE set_reset_button);

public:
  SpinBox(QWidget* parent = 0);

  // QSpinBox
  QString textFromValue(int val) const;

  // ExtendedEditor
  bool is_empty() const { return text().isEmpty() || text() == "0"; }
  void set_focus() { QSpinBox::setFocus(); }
  QString text() const { return QSpinBox::text(); }
  void set_text(const QString& text) { QSpinBox::setValue(text.toInt()); }

protected:
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);

signals:
  void Reset();
};

#endif // LINEEDIT_H

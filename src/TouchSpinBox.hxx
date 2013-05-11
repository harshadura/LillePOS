/* Copyright 2010, Martin Koller, kollix@aon.at

  This file is part of LillePOS.

  LillePOS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LillePOS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LillePOS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _TouchSpinBox_H_
#define _TouchSpinBox_H_

#include <QWidget>
#include <QSpinBox>
#include <QDate>

//--------------------------------------------------------------------------------

class TouchSpinner : public QSpinBox
{
  Q_OBJECT

  public:
    TouchSpinner(QWidget *parent)
      : QSpinBox(parent), monthMode(false)
    {
    }

    void setMonthMode() { monthMode = true; }

  protected:
    virtual QString textFromValue(int value) const
    {
      if ( !monthMode )
        return QSpinBox::textFromValue(value);

      return QDate::longMonthName(value);
    }

    virtual int valueFromText(const QString &text) const
    {
      if ( !monthMode )
        return QSpinBox::valueFromText(text);

      for (int i = 1; i <= 12; i++)
        if ( QDate::longMonthName(i) == text )
          return i;

      return 1;
    }

  private:
    bool monthMode;
};

//--------------------------------------------------------------------------------

#include <ui_TouchSpinBox.h>

class TouchSpinBox : public QWidget
{
  Q_OBJECT

  public:
    TouchSpinBox(QWidget *parent);

    void setValue(int v);
    void setValue(int v, int min, int max);
    int getValue() const;

    void setMonthMode();

    void setEditable(bool editable);

  signals:
    void valueChanged();
    void plusClicked();
    void minusClicked();

  private slots:
    void plusSlot();
    void minusSlot();

  private:
    Ui::TouchSpinBox ui;
};

#endif

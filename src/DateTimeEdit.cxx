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

#include <DateTimeEdit.hxx>

//--------------------------------------------------------------------------------

DateTimeEdit::DateTimeEdit(QWidget *parent, const QDateTime &dt)
  : QDialog(parent)
{
  ui.setupUi(this);

  connect(ui.month, SIGNAL(valueChanged()), this, SLOT(monthChanged()));  // adapt for max day in month

  ui.year->setValue(dt.date().year(), 2010, 2040);
  ui.month->setValue(dt.date().month(), 1, 12);
  ui.day->setValue(dt.date().day(), 1, dt.date().daysInMonth());
  ui.hour->setValue(dt.time().hour(), 0, 23);
  ui.minute->setValue(dt.time().minute(), 0, 59);
}

//--------------------------------------------------------------------------------

QDateTime DateTimeEdit::getDateTime() const
{
  return QDateTime(QDate(ui.year->getValue(), ui.month->getValue(), ui.day->getValue()),
                   QTime(ui.hour->getValue(), ui.minute->getValue()));
}

//--------------------------------------------------------------------------------

void DateTimeEdit::monthChanged()
{
  int maxDay = QDate(ui.year->getValue(), ui.month->getValue(), 1).daysInMonth();
  ui.day->setValue(ui.day->getValue() > maxDay ? maxDay : ui.day->getValue(), 1, maxDay);
}

//--------------------------------------------------------------------------------

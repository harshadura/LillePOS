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

#include <MonthSelectorDialog.hxx>

//--------------------------------------------------------------------------------

MonthSelectorDialog::MonthSelectorDialog(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  for (int i = 1; i <= 12; i++)
  {
    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(QSize(50, 40));
    item->setData(Qt::DisplayRole, QDate::longMonthName(i));
    ui.monthListWidget->addItem(item);
  }

  ui.monthListWidget->setCurrentRow(QDate::currentDate().month() - 1);
  ui.monthListWidget->scrollToItem(ui.monthListWidget->currentItem());
  ui.year->setValue(QDate::currentDate().year());
}

//--------------------------------------------------------------------------------

QDate MonthSelectorDialog::getDate() const
{
  return QDate(ui.year->value(), ui.monthListWidget->currentRow() + 1, 1);
}

//--------------------------------------------------------------------------------

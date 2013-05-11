/* Copyright 2010,2011 Martin Koller, kollix@aon.at

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

#include <DaySelectorDialog.hxx>
#include <QButtonGroup>

//--------------------------------------------------------------------------------

DaySelectorDialog::DaySelectorDialog(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  ui.yearSpinBox->setValue(QDate::currentDate().year(), 2010, 2040);
  ui.yearSpinBox->setEditable(false);

  ui.monthSpinBox->setValue(QDate::currentDate().month(), 1, 12);
  ui.monthSpinBox->setMonthMode();
  ui.monthSpinBox->setEditable(false);

  connect(ui.yearSpinBox, SIGNAL(plusClicked()), ui.calendarWidget, SLOT(showNextYear()));
  connect(ui.yearSpinBox, SIGNAL(minusClicked()), ui.calendarWidget, SLOT(showPreviousYear()));
  connect(ui.monthSpinBox, SIGNAL(plusClicked()), ui.calendarWidget, SLOT(showNextMonth()));
  connect(ui.monthSpinBox, SIGNAL(minusClicked()), ui.calendarWidget, SLOT(showPreviousMonth()));

  connect(ui.calendarWidget, SIGNAL(selectionChanged()), this, SLOT(calendarSelectionChanged()));

  connect(ui.buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(rangeChanged()));
}

//--------------------------------------------------------------------------------

void DaySelectorDialog::rangeChanged()
{
  ui.yearSpinBox->setEnabled(true);
  ui.monthSpinBox->setEnabled(true);
  ui.calendarWidget->setEnabled(true);

  switch ( getRange() )
  {
    case ALL:
    {
      ui.yearSpinBox->setEnabled(false);
      // fallthrough
    }
    case YEAR:
    {
      ui.monthSpinBox->setEnabled(false);
      // fallthrough
    }
    case MONTH:
    {
      ui.calendarWidget->setEnabled(false);
      // fallthrough
    }
    case DAY:
    {
      // nothing to disable
    }
  }
}

//--------------------------------------------------------------------------------

DaySelectorDialog::Range DaySelectorDialog::getRange() const
{
  if ( ui.dailyReport->isChecked() )
    return DAY;
  else if ( ui.monthlyReport->isChecked() )
    return MONTH;
  else if ( ui.yearlyReport->isChecked() )
    return YEAR;
  else
    return ALL;
}

//--------------------------------------------------------------------------------

DaySelectorDialog::ReportType DaySelectorDialog::getReportType() const
{
  if ( ui.turnover->isChecked() )
    return TURNOVER;
  else
    return STATISTICS;
}

//--------------------------------------------------------------------------------

bool DaySelectorDialog::doBackup() const
{
  return ui.makeBackup->isChecked();
}

//--------------------------------------------------------------------------------

QDate DaySelectorDialog::getDate() const
{
  switch ( getRange() )
  {
    case DAY: return ui.calendarWidget->selectedDate();
    case MONTH: return QDate(ui.calendarWidget->yearShown(), ui.calendarWidget->monthShown(), 1);
    case YEAR: return QDate(ui.calendarWidget->yearShown(), 1, 1);
    default: return QDate();
  }
}

//--------------------------------------------------------------------------------

void DaySelectorDialog::calendarSelectionChanged()
{
  ui.yearSpinBox->setValue(getDate().year());
  ui.monthSpinBox->setValue(getDate().month());
}

//--------------------------------------------------------------------------------

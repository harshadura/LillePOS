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

#ifndef _DaySelectorDialog_H_
#define _DaySelectorDialog_H_

#include <QDialog>
#include <QDate>

#include <ui_DaySelectorDialog.h>

class DaySelectorDialog : public QDialog
{
  Q_OBJECT

  public:
    DaySelectorDialog(QWidget *parent);

    QDate getDate() const;

    enum Range { DAY, MONTH, YEAR, ALL };
    Range getRange() const;

    enum ReportType { TURNOVER, STATISTICS };
    ReportType getReportType() const;

    // return the "do also a backup" flag
    bool doBackup() const;

  private slots:
    void calendarSelectionChanged();
    void rangeChanged();

  private:
    Ui::DaySelectorDialog ui;
};

#endif

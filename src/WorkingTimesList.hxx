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

#ifndef _WorkingTimesList_H_
#define _WorkingTimesList_H_

#include <QDialog>
class QSqlTableModel;

#include <ui_WorkingTimesList.h>

class WorkingTimesList : public QDialog
{
  Q_OBJECT

  public:
    WorkingTimesList(QWidget *parent, int employeeId);

  private slots:
    void plusSlot();
    void minusSlot();
    void editSlot();

  private:
    int employee;
    Ui::WorkingTimesList ui;
    QSqlTableModel *model;
};

#endif

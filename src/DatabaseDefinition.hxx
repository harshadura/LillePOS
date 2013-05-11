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

#ifndef _DatabaseDefinition_H_
#define _DatabaseDefinition_H_

#include <QDialog>

#include <ui_DatabaseDefinition.h>

class DatabaseDefinition : public QDialog
{
  Q_OBJECT

  public:
    DatabaseDefinition(QWidget *parent);

    QString getDbType() const;
    QString getUserName() const;
    QString getPassword() const;
    QString getHostName() const;

  private slots:

  private:
    Ui::DatabaseDefinition ui;
};

#endif

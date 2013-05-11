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

#include <DatabaseDefinition.hxx>

//--------------------------------------------------------------------------------

DatabaseDefinition::DatabaseDefinition(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getDbType() const
{
  if ( ui.sqlite->isChecked() )
    return QLatin1String("QSQLITE");
  else
    return QLatin1String("QMYSQL");
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getUserName() const
{
  return ui.username->text();
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getPassword() const
{
  return ui.password->text();
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getHostName() const
{
  return ui.hostname->text();
}

//--------------------------------------------------------------------------------

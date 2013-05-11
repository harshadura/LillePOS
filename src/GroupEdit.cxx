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

#include <GroupEdit.hxx>

#include <QSqlQuery>

//--------------------------------------------------------------------------------

GroupEdit::GroupEdit(QWidget *parent, int theId)
  : QDialog(parent), id(theId)
{
  ui.setupUi(this);

  categoriesModel.setQuery("SELECT id, name FROM categories");
  ui.categoryComboBox->setModel(&categoriesModel);
  ui.categoryComboBox->setModelColumn(1);  // show name

  if ( id != -1 )
  {
    QSqlQuery query(QString("SELECT name,category,visible FROM groups WHERE id=%1").arg(id));
    query.next();

    ui.name->setText(query.value(0).toString());
    ui.visibleCheckBox->setChecked(query.value(2).toBool());

    int i;
    for (i = 0; i < categoriesModel.rowCount(); i++)
      if ( query.value(1).toInt() == categoriesModel.data(categoriesModel.index(i, 0), Qt::DisplayRole).toInt() )
        break;

    ui.categoryComboBox->setCurrentIndex(i);
  }
}

//--------------------------------------------------------------------------------

void GroupEdit::accept()
{
  QSqlQuery query;

  if ( id == -1 )  // new entry
  {
    query.exec(QString("INSERT INTO groups (name, category, visible) VALUES('%1', %2, %3)")
                       .arg(ui.name->text())
                       .arg(categoriesModel.data(categoriesModel.index(ui.categoryComboBox->currentIndex(), 0)).toInt())
                       .arg(ui.visibleCheckBox->isChecked()));
  }
  else
  {
    query.exec(QString("UPDATE groups SET name='%1',category=%2,visible=%3 WHERE id=%4")
                       .arg(ui.name->text())
                       .arg(categoriesModel.data(categoriesModel.index(ui.categoryComboBox->currentIndex(), 0)).toInt())
                       .arg(ui.visibleCheckBox->isChecked())
                       .arg(id));
  }

  QDialog::accept();
}

//--------------------------------------------------------------------------------

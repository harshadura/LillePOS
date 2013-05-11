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

#include <ProductEdit.hxx>

#include <QDoubleValidator>
#include <QSqlQuery>

//--------------------------------------------------------------------------------

ProductEdit::ProductEdit(QWidget *parent, int theId)
  : QDialog(parent), id(theId)
{
  ui.setupUi(this);
  ui.price->setValidator(new QDoubleValidator(0.0, 9999999.99, 2, this));

  groupsModel.setQuery("SELECT id, name FROM groups");
  ui.groupComboBox->setModel(&groupsModel);
  ui.groupComboBox->setModelColumn(1);  // show name

  if ( id != -1 )
  {
    QSqlQuery query(QString("SELECT `name`,`group`,`visible`,`price`,`tax` FROM products WHERE id=%1").arg(id));
    query.next();

    ui.name->setText(query.value(0).toString());
    ui.visibleCheckBox->setChecked(query.value(2).toBool());
    ui.price->setText(QString::number(query.value(3).toDouble(), 'f', 2));

    int i;
    for (i = 0; i < groupsModel.rowCount(); i++)
      if ( query.value(1).toInt() == groupsModel.data(groupsModel.index(i, 0), Qt::DisplayRole).toInt() )
        break;

    ui.groupComboBox->setCurrentIndex(i);

    if ( query.value(4).toInt() == 10 )
      ui.tax10->setChecked(true);
    else if ( query.value(4).toInt() == 20 )
      ui.tax20->setChecked(true);
  }
}

//--------------------------------------------------------------------------------

void ProductEdit::accept()
{
  QSqlQuery query;
  int tax;

  if ( ui.tax10->isChecked() )
    tax = 10;
  else if ( ui.tax20->isChecked() )
    tax = 20;
  else tax = 0;

  if ( id == -1 )  // new entry
  {
    query.exec(QString("INSERT INTO products (name, `group`, visible, price, tax) VALUES('%1', %2, %3, %4, %5)")
                       .arg(ui.name->text())
                       .arg(groupsModel.data(groupsModel.index(ui.groupComboBox->currentIndex(), 0)).toInt())
                       .arg(ui.visibleCheckBox->isChecked())
                       .arg(ui.price->text().toDouble())
                       .arg(tax));
  }
  else
  {
    query.exec(QString("UPDATE products SET name='%1', `group`=%2,visible=%3,price=%4,tax=%5 WHERE id=%6")
                       .arg(ui.name->text())
                       .arg(groupsModel.data(groupsModel.index(ui.groupComboBox->currentIndex(), 0)).toInt())
                       .arg(ui.visibleCheckBox->isChecked())
                       .arg(ui.price->text().toDouble())
                       .arg(tax)
                       .arg(id));
  }

  QDialog::accept();
}

//--------------------------------------------------------------------------------

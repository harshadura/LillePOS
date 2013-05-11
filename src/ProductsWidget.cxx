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

#include <ProductsWidget.hxx>
#include <ProductEdit.hxx>
#include <TouchDelegate.hxx>

#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

ProductsWidget::ProductsWidget(QWidget *parent)
  : QDialog(parent), newProductDialog(0)
{
  ui.setupUi(this);

  connect(ui.plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui.edit, SIGNAL(clicked()), this, SLOT(editSlot()));

  model = new QSqlRelationalTableModel(this);
  model->setTable("products");
  model->setRelation(model->fieldIndex("group"), QSqlRelation("groups", "id", "name"));
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->select();

  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Produkt"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("price"), Qt::Horizontal, tr("Preis"), Qt::DisplayRole);
  model->setHeaderData(3, Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("tax"), Qt::Horizontal, tr("MWSt"), Qt::DisplayRole);

  ui.tableView->setItemDelegate(new TouchDelegate(ui.tableView));
  ui.tableView->setModel(model);
  ui.tableView->setSortingEnabled(true);
  ui.tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui.tableView->horizontalHeader()->setResizeMode(model->fieldIndex("name"), QHeaderView::Stretch);
  ui.tableView->resizeColumnToContents(model->fieldIndex("visible"));
  ui.tableView->resizeColumnToContents(model->fieldIndex("tax"));
  ui.tableView->resizeColumnToContents(3);  // related groups-name
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void ProductsWidget::plusSlot()
{
  // reuse the "new" dialog so that the next call has already the previous
  // settings defined; makes input of a lot of products of a given group simpler
  if ( !newProductDialog )
    newProductDialog = new ProductEdit(this);

  newProductDialog->exec();

  model->select();
}

//--------------------------------------------------------------------------------

void ProductsWidget::minusSlot()
{
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Produkt löschen"),
         tr("Möchten sie das Produkt '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Produkt '%1' kann nicht gelöscht werden, da es schon in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }
}

//--------------------------------------------------------------------------------

void ProductsWidget::editSlot()
{
  QModelIndex current(ui.tableView->currentIndex());
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  ProductEdit dialog(this, model->data(model->index(row, model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted )
  {
    model->select();
    ui.tableView->resizeRowsToContents();
    ui.tableView->setCurrentIndex(current);
  }
}

//--------------------------------------------------------------------------------

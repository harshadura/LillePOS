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

#include <CategoriesWidget.hxx>
#include <TouchDelegate.hxx>

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

CategoriesWidget::CategoriesWidget(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  connect(ui.plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minus, SIGNAL(clicked()), this, SLOT(minusSlot()));

  model = new QSqlTableModel(this);
  model->setTable("categories");
  model->select();
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Kategorie"), Qt::DisplayRole);

  ui.tableView->setItemDelegate(new TouchDelegate(ui.tableView));
  ui.tableView->setModel(model);
  ui.tableView->setSortingEnabled(true);
  ui.tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui.tableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void CategoriesWidget::plusSlot()
{
  int row = model->rowCount();
  model->insertRow(row);

  ui.tableView->setCurrentIndex(model->index(row, 1));
  ui.tableView->edit(model->index(row, 1));
}

//--------------------------------------------------------------------------------

void CategoriesWidget::minusSlot()
{
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Kategorie löschen"),
         tr("Möchten sie die Kategorie '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Kategorie '%1' kann nicht gelöscht werden, da sie noch in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

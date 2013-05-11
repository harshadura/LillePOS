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

#include <GroupsWidget.hxx>
#include <GroupEdit.hxx>
#include <TouchDelegate.hxx>

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRelation>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

GroupsWidget::GroupsWidget(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  connect(ui.plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui.edit, SIGNAL(clicked()), this, SLOT(editSlot()));

  model = new QSqlRelationalTableModel(this);
  model->setTable("groups");
  model->setRelation(model->fieldIndex("category"), QSqlRelation("categories", "id", "name"));
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->select();

  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);
  model->setHeaderData(3, Qt::Horizontal, tr("Kategorie"), Qt::DisplayRole);

  ui.tableView->setItemDelegate(new TouchDelegate(ui.tableView));
  ui.tableView->setModel(model);
  ui.tableView->setSortingEnabled(true);
  ui.tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui.tableView->setColumnWidth(model->fieldIndex("name"), 250);
  ui.tableView->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void GroupsWidget::plusSlot()
{
  GroupEdit dialog(this);
  dialog.exec();

  model->select();
}

//--------------------------------------------------------------------------------

void GroupsWidget::minusSlot()
{
  /*  ... this is for multi selection
  QModelIndexList selected = ui.tableView->selectionModel()->selectedIndexes();
  QMap<int, bool> rows;  // we need a row only once
  for (int i = 0; i < selected.count(); i++)
    rows[selected[i].row()] = true;

  QList<int> selectedRows = rows.keys();

  for (int i = 0; i < selectedRows.count(); i++)
  {
    if ( !model->removeRow(selectedRows[i]) )
    {
      QMessageBox::information(this, tr("Löschen nicht möglich"),
          tr("Gruppe %1 kann nicht gelöscht werden, da sie noch in Verwendung ist")
             .arg(model->data(model->index(selectedRows[i], 1)).toString()));
    }
  }
  */
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Gruppe löschen"),
         tr("Möchten sie die Gruppe '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Gruppe '%1' kann nicht gelöscht werden, da sie noch in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }
}

//--------------------------------------------------------------------------------

void GroupsWidget::editSlot()
{
  QModelIndex current(ui.tableView->currentIndex());
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  GroupEdit dialog(this, model->data(model->index(row, model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted )
  {
    model->select();
    ui.tableView->resizeRowsToContents();
    ui.tableView->setCurrentIndex(current);
  }
}

//--------------------------------------------------------------------------------

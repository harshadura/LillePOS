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

#include <EmployeesWidget.hxx>
#include <TouchDelegate.hxx>

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

EmployeesWidget::EmployeesWidget(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  connect(ui.plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minus, SIGNAL(clicked()), this, SLOT(minusSlot()));

  model = new QSqlTableModel(this);
  model->setTable("employees");
  model->select();
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Mitarbeiter"), Qt::DisplayRole);

  ui.tableView->setItemDelegate(new TouchDelegate(ui.tableView));
  ui.tableView->setModel(model);
  ui.tableView->setSortingEnabled(true);
  ui.tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui.tableView->setColumnHidden(model->fieldIndex("start"), true);
  ui.tableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void EmployeesWidget::plusSlot()
{
  int row = model->rowCount();
  model->insertRow(row);

  ui.tableView->setCurrentIndex(model->index(row, 1));
  ui.tableView->edit(model->index(row, 1));
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void EmployeesWidget::minusSlot()
{
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Mitarbeiter löschen"),
         tr("Möchten sie den Mitarbeiter '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Mitarbeiter '%1' kann nicht gelöscht werden, da er noch Arbeitszeiten eingetragen hat")
           .arg(model->data(model->index(row, 1)).toString()));
  }
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

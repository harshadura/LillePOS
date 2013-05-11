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

#include <WorkingTimesList.hxx>
#include <DateTimeEdit.hxx>
#include <TouchDelegate.hxx>

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>

//--------------------------------------------------------------------------------

class Model : public QSqlTableModel
{
  public:
    Model(QObject *parent) : QSqlTableModel(parent) {}

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
      if ( (role != Qt::DisplayRole) || (index.column() == 0) )
        return QSqlTableModel::data(index, role);

      QDateTime dt = QSqlTableModel::data(index, role).toDateTime();
      return QVariant(dt.date().toString() + "    " + dt.time().toString("hh:mm"));
    }
};

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

WorkingTimesList::WorkingTimesList(QWidget *parent, int employeeId)
  : QDialog(parent), employee(employeeId)
{
  ui.setupUi(this);

  connect(ui.plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui.edit, SIGNAL(clicked()), this, SLOT(editSlot()));

  QSqlQuery query(QString("SELECT name FROM employees WHERE id=%1").arg(employeeId));
  query.next();
  ui.employeeLabel->setText(query.value(0).toString());

  model = new Model(this);
  model->setTable("workingTimes");
  model->setFilter(QString("employee=%1").arg(employeeId));
  model->setHeaderData(1, Qt::Horizontal, tr("Start"), Qt::DisplayRole);
  model->setHeaderData(2, Qt::Horizontal, tr("Ende"), Qt::DisplayRole);
  model->select();
  model->setEditStrategy(QSqlTableModel::OnFieldChange);

  ui.tableView->setItemDelegate(new TouchDelegate(this));
  ui.tableView->setModel(model);
  ui.tableView->setColumnHidden(0, true);  // ID
  ui.tableView->sortByColumn(1, Qt::AscendingOrder);
  ui.tableView->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
  ui.tableView->resizeColumnToContents(1);
  ui.tableView->resizeColumnToContents(2);
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void WorkingTimesList::plusSlot()
{
  model->insertRows(model->rowCount(), 1);
  model->setData(model->index(model->rowCount() - 1, 0), employee);

  QDateTime dt = QDateTime::currentDateTime();
  model->setData(model->index(model->rowCount() - 1, 1), dt);
  model->setData(model->index(model->rowCount() - 1, 2), dt);
  model->submit();
  ui.tableView->resizeRowsToContents();
  ui.tableView->setCurrentIndex(model->index(model->rowCount() - 1, 1));
  editSlot();
}

//--------------------------------------------------------------------------------

void WorkingTimesList::minusSlot()
{
  int row = ui.tableView->currentIndex().row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Zeile löschen"),
         tr("Möchten sie die Zeile '%1' wirklich löschen ?").arg(row+1),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  model->removeRow(row);
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void WorkingTimesList::editSlot()
{
  DateTimeEdit dialog(this, model->data(ui.tableView->currentIndex(), Qt::EditRole).toDateTime());
  if ( dialog.exec() == QDialog::Rejected )
    return;

  QDateTime dt = dialog.getDateTime();
  model->setData(ui.tableView->currentIndex(), dt);
  ui.tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

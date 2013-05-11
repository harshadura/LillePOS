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

#include <ClosedTickets.hxx>
#include <MainWidget.hxx>

#include <QDateTime>
#include <QSqlQuery>
#include <QMessageBox>
#include <QIntValidator>

//--------------------------------------------------------------------------------

ClosedTickets::ClosedTickets(MainWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  ui.treeWidget->setColumnHidden(5, true);  // hide sortDate column only used for date/time sorting

  connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
          this, SLOT(itemClicked(QTreeWidgetItem *, int)));
  connect(ui.print, SIGNAL(clicked()), this, SLOT(printInvoice()));
  connect(ui.change, SIGNAL(clicked()), this, SLOT(changeTicket()));
  connect(ui.showPrivate, SIGNAL(toggled(bool)), this, SLOT(showPrivate(bool)));
  connect(ui.showAllDays, SIGNAL(toggled(bool)), this, SLOT(fillTree()));

  ui.searchInvoice->setValidator(new QIntValidator(1, 999999999, this));
  connect(ui.searchInvoice, SIGNAL(textChanged(const QString &)), this, SLOT(searchInvoice(const QString &)));

  fillTree();
}

//--------------------------------------------------------------------------------

void ClosedTickets::fillTree()
{
  ui.treeWidget->clear();

  QString select("SELECT tickets.id, `invoiceNum`, `timestamp`, `table`, `payedBy`, SUM(orders.count * orders.price) FROM tickets"
                  " LEFT JOIN orders on orders.ticketId=tickets.id");

  if ( !ui.showAllDays->isChecked() )
    select += QString(" WHERE DATE(tickets.timestamp)='%1'").arg(QDate::currentDate().toString(Qt::ISODate));

  select += " GROUP BY tickets.id";

  QSqlQuery query(select);

  while ( query.next() )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setSizeHint(0, QSize(50, 50));
    item->setData(0, Qt::UserRole, query.value(0).toInt());  // id
    item->setData(0, Qt::DisplayRole, query.value(1).toInt());  // invoiceNum
    item->setData(0, Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));

    item->setData(1, Qt::DisplayRole, query.value(2).toDateTime().toString(Qt::SystemLocaleShortDate));  // timestamp
    item->setData(1, Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));
    item->setData(5, Qt::DisplayRole, query.value(2).toDateTime().toString(Qt::ISODate));  // timestamp in sortable format

    int table = query.value(3).toInt();
    item->setData(2, Qt::DisplayRole, (table == -1) ? tr("Mitnehmen") : QString::number(table));
    item->setData(2, Qt::TextAlignmentRole, QVariant(Qt::AlignCenter));

    int payedBy = query.value(4).toInt();
    item->setData(3, Qt::UserRole, payedBy);
    item->setData(3, Qt::DisplayRole, MainWidget::getPayedByText(payedBy));
    item->setData(3, Qt::TextAlignmentRole, QVariant(Qt::AlignCenter));

    item->setData(4, Qt::DisplayRole, QString::number(query.value(5).toDouble(), 'f', 2));
    item->setData(4, Qt::TextAlignmentRole, QVariant(Qt::AlignRight | Qt::AlignVCenter));

    ui.treeWidget->addTopLevelItem(item);
  }
  ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);
  ui.treeWidget->scrollToBottom();

  ui.treeWidget->resizeColumnToContents(1);

  showPrivate(ui.showPrivate->isChecked());
}

//--------------------------------------------------------------------------------

void ClosedTickets::itemClicked(QTreeWidgetItem * item, int column)
{
  if ( column != 3 )
    return;

  int ticket = item->data(0, Qt::UserRole).toInt();
  int payedBy = item->data(3, Qt::UserRole).toInt();  // toggle cash/atm

  if ( payedBy == PAYED_BY_CASH )
    payedBy = PAYED_BY_ATM;
  else if ( payedBy == PAYED_BY_ATM )
    payedBy = PAYED_BY_CASH;
  else
    return;

  if ( QMessageBox::question(this, tr("Zahlart ändern"),
                             tr("Wollen sie die Zahlart wirklich auf '%1' ändern?")
                                .arg(MainWidget::getPayedByText(payedBy)),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  QSqlQuery query(QString("UPDATE tickets SET payedBy=%2 WHERE id=%1").arg(ticket).arg(payedBy));
  item->setData(3, Qt::UserRole, payedBy);
  item->setData(3, Qt::DisplayRole, MainWidget::getPayedByText(payedBy));
}

//--------------------------------------------------------------------------------

int ClosedTickets::getTicket() const
{
  QList<QTreeWidgetItem*> selected = ui.treeWidget->selectedItems();

  if ( selected.isEmpty() )
    return 0;

  return selected[0]->data(0, Qt::UserRole).toInt();
}

//--------------------------------------------------------------------------------

void ClosedTickets::printInvoice()
{
  QList<QTreeWidgetItem*> selected = ui.treeWidget->selectedItems();

  if ( selected.isEmpty() )
    return;

  int ticket = selected[0]->data(0, Qt::UserRole).toInt();
  qobject_cast<MainWidget*>(parent())->printInvoice(ticket);
}

//--------------------------------------------------------------------------------

void ClosedTickets::changeTicket()
{
  QList<QTreeWidgetItem*> selected = ui.treeWidget->selectedItems();

  if ( selected.isEmpty() )
  {
    accept();
    return;
  }

  done(CHANGE_TICKET);
}

//--------------------------------------------------------------------------------

void ClosedTickets::searchInvoice(const QString &text)
{
  // show only matching items

  for (int i = 0; i < ui.treeWidget->topLevelItemCount(); i++)
    ui.treeWidget->topLevelItem(i)->setHidden(true);

  QList<QTreeWidgetItem *> items = ui.treeWidget->findItems(text, Qt::MatchStartsWith, 0);
  foreach (QTreeWidgetItem *item, items)
    item->setHidden(false);
}

//--------------------------------------------------------------------------------

void ClosedTickets::showPrivate(bool priv)
{
  // show also private/advertising items

  for (int i = 0; i < ui.treeWidget->topLevelItemCount(); i++)
  {
    if ( priv )
    {
      ui.treeWidget->topLevelItem(i)->setHidden(false);
    }
    else
    {
      int invoiceNum = ui.treeWidget->topLevelItem(i)->data(0, Qt::DisplayRole).toInt();
      ui.treeWidget->topLevelItem(i)->setHidden(invoiceNum == 0);
    }
  }

  if ( priv )
    ui.treeWidget->sortByColumn(5, Qt::AscendingOrder);  // sort by date to see somehow correct order
  else
    ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);
}

//--------------------------------------------------------------------------------

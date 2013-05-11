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

#include <OpenTicketsListWidget.hxx>
#include <OpenTicketWidget.hxx>

#include <QHBoxLayout>
#include <QSqlQuery>

//--------------------------------------------------------------------------------

OpenTicketsListWidget::OpenTicketsListWidget(QWidget *parent)
  : QScrollArea(parent)
{
  setWidgetResizable(true);

  QWidget *contentsWidget = new QWidget(this);
  contentsWidget->resize(1348, 682);
  new QHBoxLayout(contentsWidget);

  setWidget(contentsWidget);
}

//--------------------------------------------------------------------------------

void OpenTicketsListWidget::refreshTickets()
{
  // clear previous contents
  QLayoutItem *item;
  while ( (item = widget()->layout()->takeAt(0)) )
  {
    delete item->widget();
    delete item;
  }

  QSqlQuery query("SELECT `id`, `table` FROM tickets WHERE open=1 ORDER BY `table`");

  while ( query.next() )
  {
    OpenTicketWidget *ticketTree = new OpenTicketWidget(this, query.value(0).toInt(), query.value(1).toInt());
    widget()->layout()->addWidget(ticketTree);
  }

  static_cast<QHBoxLayout*>(widget()->layout())->addStretch();
}

//--------------------------------------------------------------------------------

void OpenTicketsListWidget::selectTicket(int id)
{
  for (int i = 0; i < widget()->layout()->count(); i++)
  {
    QLayoutItem *item = widget()->layout()->itemAt(i);
    if ( item->widget() && (static_cast<OpenTicketWidget*>(item->widget())->getId() == id) )
    {
      static_cast<OpenTicketWidget*>(item->widget())->setSelected(true);
      break;
    }
  }
}

//--------------------------------------------------------------------------------

QList<int> OpenTicketsListWidget::getSelectedTickets() const
{
  QList<int> list;

  for (int i = 0; i < widget()->layout()->count(); i++)
  {
    QLayoutItem *item = widget()->layout()->itemAt(i);
    if ( item->widget() && static_cast<OpenTicketWidget*>(item->widget())->isSelected() )
      list.append(static_cast<OpenTicketWidget*>(item->widget())->getId());
  }

  return list;
}

//--------------------------------------------------------------------------------

QList<int> OpenTicketsListWidget::getTickets() const
{
  QList<int> list;

  for (int i = 0; i < widget()->layout()->count(); i++)
  {
    QLayoutItem *item = widget()->layout()->itemAt(i);
    if ( item->widget() )
      list.append(static_cast<OpenTicketWidget*>(item->widget())->getId());
  }

  return list;
}

//--------------------------------------------------------------------------------

int OpenTicketsListWidget::getTableOfTicket(int id)
{
  for (int i = 0; i < widget()->layout()->count(); i++)
  {
    QLayoutItem *item = widget()->layout()->itemAt(i);
    if ( item->widget() && (static_cast<OpenTicketWidget*>(item->widget())->getId() == id) )
    {
      return static_cast<OpenTicketWidget*>(item->widget())->getTable();
    }
  }

  return 0;
}

//--------------------------------------------------------------------------------

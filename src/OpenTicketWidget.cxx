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

#include <OpenTicketWidget.hxx>

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSqlQuery>
#include <QEvent>
#include <QPushButton>

// orderExtras.type
enum
{
  TYPE_WITH = 1,
  TYPE_WITHOUT = 0
};

//--------------------------------------------------------------------------------

TreeWidget::TreeWidget(QWidget *parent)
  : QTreeWidget(parent)
{
  setTextElideMode(Qt::ElideMiddle);
  setExpandsOnDoubleClick(false);
}

//--------------------------------------------------------------------------------

bool TreeWidget::viewportEvent(QEvent *event)
{
  if ( event->type() == QEvent::MouseButtonPress )
    emit selected();

  return QTreeWidget::viewportEvent(event);
}

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

OpenTicketWidget::OpenTicketWidget(QWidget *parent, int theId, int theTable)
  : QFrame(parent), id(theId), table(theTable), selected(false), toServeWidget(0)
{
  setAutoFillBackground(true);
  setFixedWidth(270);
  setFrameShape(QFrame::Box);
  setSelected(selected);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  QHBoxLayout *hbox = new QHBoxLayout;

  QLabel *label = new QLabel(this);
  label->setAlignment(Qt::AlignLeft);

  if ( table == -1 )
    label->setText(tr("<b>Mitnehmen</b>"));
  else
    label->setText(tr("<b>Tisch: %1</b>").arg(table));

  hbox->addWidget(label);

  QTreeWidget *ticketTree = new TreeWidget(this);
  QTreeWidgetItem *headerItem = ticketTree->headerItem();
  headerItem->setText(0, tr("Anz"));
  headerItem->setText(1, tr("Produkt"));
  headerItem->setText(2, tr("je"));  // price per piece

  ticketTree->header()->setStretchLastSection(false);
  //ticketTree->header()->setResizeMode(0, QHeaderView::ResizeToContents);
  ticketTree->header()->setResizeMode(1, QHeaderView::Stretch);
  ticketTree->header()->setResizeMode(2, QHeaderView::ResizeToContents);
  ticketTree->header()->resizeSection(0, 33);
  ticketTree->setRootIsDecorated(false);
  ticketTree->setSelectionMode(QAbstractItemView::NoSelection);

  connect(ticketTree, SIGNAL(selected()), this, SLOT(selectedSlot()));

  QSqlQuery orders(QString("SELECT orders.count, products.name, orders.price, orders.id FROM orders "
                           " LEFT JOIN products ON orders.product=products.id"
                           " WHERE orders.ticketId=%1")
                           .arg(id));
  double sum = 0;
  while ( orders.next() )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setData(0, Qt::DisplayRole, orders.value(0).toInt());    // count
    item->setData(1, Qt::DisplayRole, orders.value(1).toString()); // product name
    item->setData(2, Qt::DisplayRole, QString::number(orders.value(2).toDouble(), 'f', 2)); // product price
    item->setTextAlignment(2, Qt::AlignRight);
    ticketTree->addTopLevelItem(item);

    sum += (orders.value(0).toInt() * orders.value(2).toDouble());

    // check for orderExtras
    QSqlQuery extras(QString("SELECT orderExtras.type, products.name FROM orderExtras "
                             " LEFT JOIN products ON orderExtras.product=products.id"
                             " WHERE orderId=%1")
                             .arg(orders.value(3).toInt()));

    while ( extras.next() )
    {
      QTreeWidgetItem *child = new QTreeWidgetItem(item);
      child->setData(0, Qt::DisplayRole, (extras.value(0).toInt() == TYPE_WITH) ? "+" : "-");
      child->setData(1, Qt::DisplayRole, extras.value(1).toString());

      item->setExpanded(true);
    }
  }

  QLabel *sumLabel = new QLabel(QString("Summe: %1").arg(QString::number(sum, 'f', 2)), this);
  sumLabel->setAlignment(Qt::AlignRight);
  hbox->addWidget(sumLabel);
  vbox->addLayout(hbox);
  vbox->addWidget(ticketTree);

  //--------------------------------
  // show unprinted orders
  orders.exec(QString("SELECT (orders.count - orders.printed), products.name, orders.id FROM orders "
                      " LEFT JOIN products ON orders.product=products.id"
                      " WHERE orders.ticketId=%1 AND (orders.count > orders.printed)")
                      .arg(id));

  if ( orders.next() )
  {
    toServeWidget = new QWidget(this);
    QVBoxLayout *vb = new QVBoxLayout(toServeWidget);
    vb->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *hb = new QHBoxLayout;
    hb->addWidget(new QLabel(tr("Zu Servieren"), this));
    QPushButton *servedButton = new QPushButton(tr("Serviert"));
    hb->addWidget(servedButton);
    connect(servedButton, SIGNAL(clicked()), this, SLOT(ordersServed()));
    vb->addLayout(hb);

    QTreeWidget *toServe = new TreeWidget(this);
    QTreeWidgetItem *headerItem = toServe->headerItem();
    headerItem->setText(0, tr("Anz"));
    headerItem->setText(1, tr("Produkt"));

    toServe->setColumnCount(2);
    toServe->header()->resizeSection(0, 40);
    toServe->setRootIsDecorated(false);
    toServe->setSelectionMode(QAbstractItemView::NoSelection);

    do
    {
      QTreeWidgetItem *item = new QTreeWidgetItem;
      item->setData(0, Qt::DisplayRole, orders.value(0).toInt());    // count
      item->setData(1, Qt::DisplayRole, orders.value(1).toString()); // product name
      toServe->addTopLevelItem(item);

      // check for orderExtras
      QSqlQuery extras(QString("SELECT orderExtras.type, products.name FROM orderExtras "
                               " LEFT JOIN products ON orderExtras.product=products.id"
                               " WHERE orderId=%1")
                               .arg(orders.value(2).toInt()));

      while ( extras.next() )
      {
        QTreeWidgetItem *child = new QTreeWidgetItem(item);
        child->setData(0, Qt::DisplayRole, (extras.value(0).toInt() == TYPE_WITH) ? "+" : "-");
        child->setData(1, Qt::DisplayRole, extras.value(1).toString());

        item->setExpanded(true);
      }
    }
    while ( orders.next() );

    vb->addWidget(toServe);

    vbox->addWidget(toServeWidget);

    connect(toServe, SIGNAL(selected()), this, SLOT(selectedSlot()));
  }
}

//--------------------------------------------------------------------------------

void OpenTicketWidget::setSelected(bool sel)
{
  selected = sel;

  setBackgroundRole(selected ? QPalette::Highlight : QPalette::AlternateBase);
}

//--------------------------------------------------------------------------------

void OpenTicketWidget::selectedSlot()
{
  setSelected(!selected);
}

//--------------------------------------------------------------------------------
// mark unserverd orders as served

void OpenTicketWidget::ordersServed()
{
  QSqlQuery orders(QString("UPDATE orders SET printed=count WHERE ticketId=%1").arg(id));
  toServeWidget->hide();
}

//--------------------------------------------------------------------------------

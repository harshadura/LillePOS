/* Copyright 2010-2013, Martin Koller, kollix@aon.at

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

#include <MainWidget.hxx>
#include <SettingsDialog.hxx>
#include <TableSelection.hxx>
#include <ClosedTickets.hxx>
#include <DaySelectorDialog.hxx>
#include <PictureView.hxx>
#include <WorkingTimes.hxx>
#include <ReportView.hxx>
#include <DatabaseDefinition.hxx>
#include <FinishTicketDialog.hxx>
#include <Backup.hxx>

#include <QVBoxLayout>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSettings>
#include <QDateTime>
#include <QScrollBar>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPrintDialog>
#include <QPainter>
#include <QFrame>
#include <QHeaderView>
#include <QDesktopWidget>
#include <QUrl>
#include <QProcess>
#include <QTemporaryFile>
#include <QPixmap>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

//--------------------------------------------------------------------------------
// NOTE: Tables are either -1 (to go) or >= 1 (table num). table==0 is invalid

// we must store the price per orders entry as the price can change over time
// for a product. But we need to know exactly what price it had at the time
// of the ticket

//--------------------------------------------------------------------------------

enum
{
  PRODUCT_ID    = Qt::UserRole,
  PRODUCT_PRICE = Qt::UserRole + 1,
  EXTRA_TYPE    = Qt::UserRole + 2,    // orderExtras.type: 1=with, 0=without
  ORDER_ID      = Qt::UserRole + 3     // orders.id for the extra items
};

// orderExtras.type
enum
{
  TYPE_WITH = 1,
  TYPE_WITHOUT = 0
};

#define MIN_HEIGHT 60

const int PIC_BUTTONS_PER_ROW = 6;

//--------------------------------------------------------------------------------

MainWidget::MainWidget()
  : menu(0), tableSelection(0), numTables(0), currentTable(0), currentTicket(0), noPrinter(false)
{
  QFont font = QApplication::font();
  font.setPointSize(11);
  QApplication::setFont(font);

  ui.setupUi(this);

  if ( QApplication::desktop()->width() < 1200 )
  {
    ui.groupBox->setFixedWidth(120);
    ui.groups_groupBox->setFixedWidth(600);
    ui.plusButton->setMinimumWidth(0);
    ui.minusButton->setMinimumWidth(0);
    ui.withButton->setMinimumWidth(0);
    ui.withoutButton->setMinimumWidth(0);
  }

  // page 0
  connect(ui.newTicket, SIGNAL(clicked()), this, SLOT(newTicket()));
  connect(ui.changeTicket, SIGNAL(clicked()), this, SLOT(changeTicket()));
  connect(ui.reopenTicket, SIGNAL(clicked()), this, SLOT(reopenTicket()));
  connect(ui.voidTicket, SIGNAL(clicked()), this, SLOT(voidTicket()));
  connect(ui.payTicket, SIGNAL(clicked()), this, SLOT(payTicket()));

  // create the menu popup
  // can not use QMenu as I can not define the height of the entries
  {
    menu = new QFrame(this, Qt::Popup);
    menu->setFrameStyle(QFrame::StyledPanel);
    menu->hide();
    QVBoxLayout *vbox = new QVBoxLayout(menu);

    QPushButton *b;
    b = new QPushButton(QIcon(":/icons/exit.png"), tr("Beenden"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(exitSlot()));
    vbox->addWidget(b);

    b = new QPushButton(QIcon(":/icons/view-fullscreen.png"), tr("Vollbild"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(fullScreenSlot()));
    vbox->addWidget(b);

    b = new QPushButton(QIcon(":/icons/settings.png"), tr("Einstellungen"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(settingsSlot()));
    vbox->addWidget(b);

    b = new QPushButton(QIcon(":/icons/clock.png"), tr("Arbeitszeiten"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(workingTimesSlot()));
    vbox->addWidget(b);

    b = new QPushButton(QIcon(":/icons/reports.png"), tr("Berichte"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(reportsSlot()));
    vbox->addWidget(b);

    connect(ui.menuButton, SIGNAL(clicked()), this, SLOT(menuSlot()));
  }

  // page 1
  ui.orderList->header()->resizeSection(0, 35);
  ui.orderList->verticalScrollBar()->setMinimumWidth(MIN_HEIGHT);

  categoriesLayout = new QVBoxLayout(ui.categoriesContentsWidget);
  groupsLayout = new QGridLayout(ui.groupsContentsWidget);

  connect(ui.plusButton, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minusButton, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeSlot()));
  connect(ui.tableButton, SIGNAL(clicked()), this, SLOT(tableSlot()));

  connect(ui.withButton, SIGNAL(clicked()), this, SLOT(withButtonSlot()));
  connect(ui.withoutButton, SIGNAL(clicked()), this, SLOT(withoutButtonSlot()));

  connect(ui.doneButton, SIGNAL(clicked()), this, SLOT(doneSlot()));
  connect(ui.printButton, SIGNAL(clicked()), this, SLOT(donePrintSlot()));
  connect(ui.payNowButton, SIGNAL(clicked()), this, SLOT(payNowSlot()));

  connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelSlot()));

  connect(&categoriesMapper, SIGNAL(mapped(int)), this, SLOT(categorySelected(int)));
  connect(&groupsMapper, SIGNAL(mapped(int)), this, SLOT(groupSelected(int)));
  connect(&productsMapper, SIGNAL(mapped(int)), this, SLOT(productSelected(int)));

  setCurrentIndex(0);

  // the pictures viewer
  {
    picturesFrame = new QFrame(ui.page1);
    picturesFrame->setAutoFillBackground(true);
    picturesFrame->setFrameStyle(QFrame::StyledPanel);
    new QHBoxLayout(picturesFrame);
    picturesFrame->hide();
    numPicturesShown = 0;

    connect(&picturesMapper, SIGNAL(mapped(int)), this, SLOT(pictureSelected(int)));

    pictureFrameArrow = new QPushButton(QIcon(":/icons/arrow-right.png"), "", ui.page1);
    pictureFrameArrow->setFixedHeight(MIN_HEIGHT);
    pictureFrameArrow->setFixedWidth(MIN_HEIGHT);
    pictureFrameArrow->setIconSize(QSize(32, 32));
    pictureFrameArrow->hide();

    connect(pictureFrameArrow, SIGNAL(clicked()), this, SLOT(movePicturesSlot()));

#if QT_VERSION >= 0x040600
    pictureAnimation = new QPropertyAnimation(picturesFrame, "pos", this);
    pictureAnimation->setDuration(1000);
    pictureAnimation->setEasingCurve(QEasingCurve::OutQuart);
#endif
  }
}

//--------------------------------------------------------------------------------

bool MainWidget::openDB(bool dbSelect)
{
  const int CURRENT_SCHEMA_VERSION = 2;

  // read global defintions (DB, num tables, ...)
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "LillePOS", "LillePOS");
  QString dbType = settings.value("DB_type").toString();

  if ( dbType.isEmpty() || dbSelect )
  {
    DatabaseDefinition dialog(this);

    if ( dialog.exec() == QDialog::Rejected )
      return false;

    dbType = dialog.getDbType();

    settings.setValue("DB_type", dbType);
    settings.setValue("DB_userName", dialog.getUserName());
    settings.setValue("DB_password", dialog.getPassword());
    settings.setValue("DB_hostName", dialog.getHostName());
  }

  QSqlDatabase db;

  // setup database connection
  if ( dbType == "QSQLITE" )
  {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("LillePOS.db");
  }
  else if ( dbType == "QMYSQL" )
  {
    QString userName = settings.value("DB_userName", "LillePOS").toString();
    QString password = settings.value("DB_password", "").toString();
    QString hostName = settings.value("DB_hostName", "localhost").toString();

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(hostName);
    db.setUserName(userName);
    db.setPassword(password);
  }

  bool ok = db.open();

  if ( !ok )
  {
    QMessageBox errorDialog;
    errorDialog.setIcon(QMessageBox::Critical);
    errorDialog.addButton(QMessageBox::Ok);
    errorDialog.setText(db.lastError().text());
    errorDialog.setWindowTitle(QObject::tr("Datenbank Verbindungsfehler"));
    errorDialog.exec();
    return false;
  }

  if ( dbType == "QMYSQL" )
  {
    QSqlQuery query("SHOW DATABASES LIKE 'LillePOS'");
    if ( !query.next() )  // db does not exist
      query.exec("CREATE DATABASE LillePOS");

    db.close();
    db.setDatabaseName("LillePOS");
    if ( !db.open() )
      return false;
  }

  QSqlQuery query("SELECT 1 FROM globals");
  if ( !query.next() )  // empty DB, create all tables
  {
    QFile f;

    if ( dbType == "QSQLITE" )
    {
      f.setFileName(":/LillePOS-sqlite.sql");
    }
    else if ( dbType == "QMYSQL" )
    {
      f.setFileName(":/LillePOS-mysql.sql");
    }

    f.open(QIODevice::ReadOnly);
    QString cmd = f.readAll();
    QStringList commands = cmd.split(';', QString::SkipEmptyParts);
    foreach (const QString &command, commands)
      query.exec(command);

    query.exec("INSERT INTO globals (name, value) VALUES('numTables', 5)");
    query.exec("INSERT INTO globals (name, value) VALUES('numPictureButtons', 0)");
    query.exec("INSERT INTO globals (name, value) VALUES('lastInvoiceNum', 0)");
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopName', '')");
    query.exec("INSERT INTO globals (name, strValue) VALUES('backupTarget', '')");

    query.exec(QString("INSERT INTO globals (name, value) VALUES('schemaVersion', %1)")
                       .arg(CURRENT_SCHEMA_VERSION));
  }
  else  // db already exists; check if we need to run an update
  {
    int schemaVersion = 1;
    query.exec("SELECT value FROM globals WHERE name='schemaVersion'");
    if ( query.next() )
      schemaVersion = query.value(0).toInt();
    else  // schemaVersion not set in globals, must be version 1
      query.exec("INSERT INTO globals (name, value) VALUES('schemaVersion', 1)");

    // run all db update scripts from the db version + 1 to what the program currently needs
    for (int i = schemaVersion + 1; i <= CURRENT_SCHEMA_VERSION; i++)
    {
      QFile f;

      if ( dbType == "QSQLITE" )
      {
        f.setFileName(QString(":/LillePOS-sqlite-update-%1.sql").arg(i));
      }
      else if ( dbType == "QMYSQL" )
      {
        f.setFileName(QString(":/LillePOS-mysql-update-%1.sql").arg(i));
      }

      if ( !f.open(QIODevice::ReadOnly) )
      {
        qWarning("could not load internal update file %s", qPrintable(f.fileName()));
        return false;  // should never happen
      }

      QString cmd = f.readAll();
      QStringList commands = cmd.split(';', QString::SkipEmptyParts);
      foreach (const QString &command, commands)
        query.exec(command);
    }

    if ( schemaVersion != CURRENT_SCHEMA_VERSION )
    {
      query.exec(QString("UPDATE globals SET value=%1 WHERE name='schemaVersion'")
                         .arg(CURRENT_SCHEMA_VERSION));
    }
  }

  if ( dbType == "QSQLITE" )
  {
    // enforce foreign key constraint
    query.exec("PRAGMA foreign_keys = 1;");
  }

  init();

  return true;
}

//--------------------------------------------------------------------------------

void MainWidget::init()
{
  QSqlQuery query("SELECT value FROM globals WHERE name='numTables'");
  query.next();
  numTables = query.value(0).toInt();

  delete tableSelection;  // previous in case numTables did change
  tableSelection = new TableSelection(this, numTables);

  query.exec("SELECT strValue FROM globals WHERE name='shopName'");
  query.next();
  shopName = query.value(0).toString();

  // fill categories
  {
    // clear previous contents
    QLayoutItem *item;
    while ( (item = categoriesLayout->takeAt(0)) )
    {
      delete item->widget();
      delete item;
    }

    query.exec("SELECT id, name FROM categories ORDER BY `name`");
    while ( query.next() )
    {
      QPushButton *b = new QPushButton(query.value(1).toString());
      b->setMinimumHeight(MIN_HEIGHT);
      categoriesLayout->addWidget(b);

      connect(b, SIGNAL(clicked()), &categoriesMapper, SLOT(map()));
      categoriesMapper.setMapping(b, query.value(0).toInt());
    }

    categoriesLayout->addStretch();
  }

  ui.openTickets->refreshTickets();

  // picture buttons
  {
    // clear previous contents
    QLayoutItem *item;
    while ( (item = ui.pictureButtonsLayout->takeAt(0)) )
    {
      delete item->widget();
      delete item;
    }

    qDeleteAll(picturesFrame->findChildren<PictureView*>());

    query.exec("SELECT value FROM globals WHERE name='numPictureButtons'");
    query.next();
    int buttons = query.value(0).toInt();

    for (int i = 0; i < buttons; i++)
    {
      query.exec(QString("SELECT strValue FROM globals WHERE name='pictureButton%1'").arg(i));
      query.next();
      QString title = query.value(0).toString();

      QPushButton *b = new QPushButton(title);
      b->setFixedHeight(MIN_HEIGHT);
      b->setCheckable(true);

      int row, column;
      row = i / PIC_BUTTONS_PER_ROW;
      column = i % PIC_BUTTONS_PER_ROW;

      ui.pictureButtonsLayout->addWidget(b, row, column);

      connect(b, SIGNAL(toggled(bool)), &picturesMapper, SLOT(map()));
      picturesMapper.setMapping(b, i);

      PictureView *view = new PictureView(picturesFrame);
      view->hide();
      QPixmap pixmap;
      pixmap.load(QString("pictureButton%1.jpg").arg(i));
      if ( pixmap.isNull() )
      {
        pixmap.load(":/icons/menu.png");
        pixmap = pixmap.scaledToWidth(QApplication::desktop()->width() / 2);
      }
      view->setPixmap(pixmap);
      query.exec(QString("SELECT strValue FROM globals WHERE name='pictureText%1'").arg(i));
      query.next();
      view->setText(query.value(0).toString());
      view->setTitle(title);

      picturesFrame->layout()->addWidget(view);
    }

    numPicturesShown = 0;
    pictureFrameArrow->hide();
  }
}

//--------------------------------------------------------------------------------

void MainWidget::newTicket()
{
  // select a table
  QPoint p(ui.newTicket->x(),
           ui.newTicket->y() - tableSelection->sizeHint().height());
  currentTable = tableSelection->exec(mapToGlobal(p));
  if ( currentTable == 0 )
    return;

  setCurrentIndex(1);

  ui.tableButton->setText(QString::number(currentTable));

  currentTicket = 0;  // a new ticket not yet in the DB
  ui.orderList->clear();
  ui.withButton->setChecked(false);
  ui.withoutButton->setChecked(false);
  clearGroupsLayout();

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void MainWidget::getSelectedTicket(int &id, int &table)
{
  id = table = 0;

  QList<int> tickets = ui.openTickets->getTickets();
  QList<int> selected =  ui.openTickets->getSelectedTickets();

  if ( selected.count() > 1 )
  {
    QMessageBox::information(this, tr("Mehrfache Selektion"), tr("Nur ein Bon darf selektiert sein"));
    return;
  }

  // if there is only 1 ticket, use this even if not selected
  if ( tickets.count() == 1 )
  {
    id = tickets[0];
    table = ui.openTickets->getTableOfTicket(id);
    return;
  }

  if ( selected.count() == 1 )
  {
    id = selected[0];
    table = ui.openTickets->getTableOfTicket(id);
    return;
  }
}

//--------------------------------------------------------------------------------

void MainWidget::changeTicket(int ticket)
{
  int table = 0;

  if ( ticket == 0 )
  {
    getSelectedTicket(ticket, table);
    if ( ticket == 0 )
      return;
  }
  else
  {
    QSqlQuery query(QString("SELECT `table`,`open` FROM tickets WHERE id=%1").arg(ticket));
    query.next();
    table = query.value(0).toInt();
    //if ( query.value(1).toBool() == false )  // already closed; reopen it
      //query.exec(QString("UPDATE tickets SET open=1 WHERE id=%1").arg(ticket));
  }

  currentTicket = ticket;
  currentTable = table;

  // fill page 1
  ui.orderList->clear();
  ui.withButton->setChecked(false);
  ui.withoutButton->setChecked(false);
  clearGroupsLayout();

  QSqlQuery orders(QString("SELECT orders.count, orders.product, orders.price, products.name, orders.id FROM orders "
                           " LEFT JOIN products ON orders.product=products.id"
                           " WHERE orders.ticketId=%1")
                           .arg(currentTicket));

  while ( orders.next() )
  {
    int orderId = orders.value(4).toInt();

    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setData(0, Qt::DisplayRole, orders.value(0).toInt());    // count
    item->setData(1, Qt::DisplayRole, orders.value(3).toString()); // product name
    item->setData(1, PRODUCT_ID, orders.value(1).toInt());
    item->setData(1, PRODUCT_PRICE, orders.value(2).toDouble());
    item->setData(1, ORDER_ID, orderId);
    item->setSizeHint(0, QSize(50, 50));
    ui.orderList->addTopLevelItem(item);

    // check for orderExtras
    QSqlQuery extras(QString("SELECT orderExtras.type, orderExtras.product, products.name FROM orderExtras"
                             " LEFT JOIN products ON orderExtras.product=products.id"
                             " WHERE orderId=%1")
                             .arg(orderId));

    while ( extras.next() )
    {
      QTreeWidgetItem *child = new QTreeWidgetItem(item);
      child->setData(0, Qt::DisplayRole, (extras.value(0).toInt() == TYPE_WITH) ? "+" : "-");
      child->setData(1, EXTRA_TYPE, extras.value(0).toInt());
      child->setData(1, PRODUCT_ID, extras.value(1).toInt());
      child->setData(1, Qt::DisplayRole, extras.value(2).toString());

      item->setExpanded(true);
    }
  }

  ui.tableButton->setText(QString::number(table));
  setCurrentIndex(1);

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void MainWidget::voidTicket()
{
  int ticket, table;
  getSelectedTicket(ticket, table);
  if ( ticket == 0 )
    return;

  if ( QMessageBox::question(this, tr("Bon löschen"), tr("Möchten sie diesen Bon wirklich löschen ?"),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  // remove ticket
  QSqlQuery query;
  query.exec(QString("DELETE FROM orderExtras WHERE orderId IN (SELECT id FROM orders WHERE ticketId=%1)").arg(ticket));
  query.exec(QString("DELETE FROM orders WHERE ticketId=%1").arg(ticket));
  query.exec(QString("DELETE FROM tickets WHERE id=%1").arg(ticket));

  // remove from display
  ui.openTickets->refreshTickets();
}

//--------------------------------------------------------------------------------

bool MainWidget::finishTicket(int ticket)
{
  // calc sum
  QSqlQuery query(QString("SELECT SUM(count * price) FROM orders WHERE ticketId=%1").arg(ticket));
  query.next();
  double sum = query.value(0).toDouble();

  query.exec(QString("SELECT invoiceNum, open FROM tickets WHERE id=%1").arg(ticket));
  query.next();
  int invoiceNum = query.value(0).toInt();  // > 0 if this is a reopened ticket
  bool reopenedTicket = query.value(1).toInt() == 0;  // already closed

  QDateTime invoiceTime = QDateTime::currentDateTime();

  FinishTicketDialog question(this, invoiceNum > 0, sum);

  int code = question.exec();
  if ( code == QDialog::Rejected )
    return false;

  if ( (code == FinishTicketDialog::WITH_PRINTOUT) || (code == FinishTicketDialog::WITHOUT_PRINTOUT) )
  {
    if ( question.getDiscount() == 20 )
    {
      query.exec(QString("UPDATE orders SET price=price*0.8 WHERE ticketId=%1").arg(ticket));
    }

    if ( invoiceNum == 0 )  // a new ticket; no need to change anything on a reopened ticket
    {
      // TODO how to make increase an atomic action (more terminals) ?
      // "update globals set value=value+1 WHERE name='lastInvoiceNum'"
      // but how to retrieve this without being interrupted by someone else ? locking ?
      query.exec("SELECT value FROM globals WHERE name='lastInvoiceNum'");
      query.next();
      invoiceNum = query.value(0).toInt();

      invoiceNum++;
      query.exec(QString("UPDATE globals SET value=%1 WHERE name='lastInvoiceNum'").arg(invoiceNum));

      query.exec(QString("UPDATE tickets SET invoiceNum=%1, timestamp='%2', open=0, payedBy=0 WHERE id=%3")
                           .arg(invoiceNum)
                           .arg(invoiceTime.toString(Qt::ISODate))
                           .arg(ticket));
    }
  }
  else  // private/employee or advertising ticket (without invoice)
  {
    int payedBy = 0;

    if ( code == FinishTicketDialog::PRIVATE_TICKET )
      payedBy = PAYED_BY_PRIVATE;
    else if ( code == FinishTicketDialog::ADVERTISING_TICKET )
      payedBy = PAYED_BY_ADVERTISING;
    else if ( code == FinishTicketDialog::EMPLOYEE_TICKET )
      payedBy = PAYED_BY_EMPLOYEE;

    if ( reopenedTicket )  // don't change timestamp on a reopened ticket
    {
      query.exec(QString("UPDATE tickets SET payedBy=%1 WHERE id=%2")
                           .arg(payedBy)
                           .arg(ticket));
    }
    else
    {
      query.exec(QString("UPDATE tickets SET timestamp='%1', open=0, payedBy=%2 WHERE id=%3")
                           .arg(invoiceTime.toString(Qt::ISODate))
                           .arg(payedBy)
                           .arg(ticket));
    }
  }

  ui.openTickets->refreshTickets();

  if ( code == FinishTicketDialog::WITH_PRINTOUT )
    printInvoice(ticket);

  return true;
}

//--------------------------------------------------------------------------------

void MainWidget::payTicket()
{
  QList<int> selected = ui.openTickets->getSelectedTickets();

  if ( selected.count() > 1 )
  {
    payGroupTicket(selected);
    return;
  }

  QList<int> tickets = ui.openTickets->getTickets();
  if ( tickets.count() == 1 )  // if there's only 1, use this
    finishTicket(tickets[0]);
  else if ( selected.count() == 1 )  // if there are more, use only if 1 is selected
    finishTicket(selected[0]);
}

//--------------------------------------------------------------------------------

void MainWidget::payGroupTicket(QList<int> tickets)
{
  if ( tickets.count() <= 1 )
    return;

  if ( QMessageBox::question(this, tr("Bons zusammenlegen"),
                             tr("Möchten sie diese %1 Bons wirklich zusammenlegen ?").arg(tickets.count()),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  int firstId = tickets[0];
  for (int i = 1; i < tickets.count(); i++)
  {
    QSqlQuery query(QString("UPDATE orders set ticketId=%1 WHERE ticketId=%2").arg(firstId).arg(tickets[i]));
    query.exec(QString("DELETE FROM tickets where id=%1").arg(tickets[i]));
  }

  // update for merged ticket
  ui.openTickets->refreshTickets();

  // select the merged one
  ui.openTickets->selectTicket(firstId);

  finishTicket(firstId);
}

//--------------------------------------------------------------------------------

void MainWidget::clearGroupsLayout()
{
  // delete all previous content
  QLayoutItem *item;
  while ( (item = groupsLayout->takeAt(0)) )
  {
    delete item->widget();
    delete item;
  }

  ui.groups_groupBox->setTitle(tr("Gruppen"));  // changes with products
}

//--------------------------------------------------------------------------------

void MainWidget::categorySelected(int category)
{
  clearGroupsLayout();

  QSqlQuery query("SELECT id, name FROM groups WHERE visible=1 AND groups.category=" + QString::number(category) +
                  " ORDER BY `name`");
  int i = 0;

  while ( query.next() )
  {
    QString text = query.value(1).toString();
    text.replace('&', "&&");
    if ( QApplication::desktop()->width() < 1200 )
    {
      int pos = text.lastIndexOf(' ');
      if ( pos > 0 )
        text.replace(pos, 1, '\n');
    }
    QPushButton *b = new QPushButton(text);
    b->setMinimumHeight(MIN_HEIGHT);
    groupsLayout->addWidget(b, i / 3, i % 3);

    connect(b, SIGNAL(clicked()), &groupsMapper, SLOT(map()));
    groupsMapper.setMapping(b, query.value(0).toInt());
    i++;
  }
  groupsLayout->setRowStretch(i / 3 + 1, 1);
}

//--------------------------------------------------------------------------------

void MainWidget::groupSelected(int group)
{
  // delete all previous content
  clearGroupsLayout();
  ui.groups_groupBox->setTitle(tr("Produkte"));  // changes with products

  QSqlQuery query("SELECT id, name FROM products WHERE visible=1 AND `group`=" + QString::number(group) +
                  " ORDER BY `name`");
  int i = 0;

  while ( query.next() )
  {
    QString text = query.value(1).toString();
    text.replace('&', "&&");
    if ( QApplication::desktop()->width() < 1200 )
    {
      int pos = text.lastIndexOf(' ');
      if ( pos > 0 )
        text.replace(pos, 1, '\n');
    }
    QPushButton *b = new QPushButton(text);
    b->setMinimumHeight(MIN_HEIGHT);
    groupsLayout->addWidget(b, i / 3, i % 3);

    connect(b, SIGNAL(clicked()), &productsMapper, SLOT(map()));
    productsMapper.setMapping(b, query.value(0).toInt());
    i++;
  }
  groupsLayout->setRowStretch(i / 3 + 1, 1);
}

//--------------------------------------------------------------------------------

void MainWidget::productSelected(int product)
{
  // if the with/without buttons are checked, we need to add this product as sub-item
  // to the currently selected one
  int extraType = -1;
  if ( ui.withButton->isChecked() )
    extraType = TYPE_WITH;
  else if ( ui.withoutButton->isChecked() )
    extraType = TYPE_WITHOUT;

  if ( extraType != -1 )
  {
    QList<QTreeWidgetItem*> selected = ui.orderList->selectedItems();

    if ( selected.isEmpty() || selected[0]->parent() )  // do not add sub-sub item
      return;

    // find product in children and change type
    bool found = false;
    for (int i = 0; i < selected[0]->childCount(); i++)
    {
      QTreeWidgetItem *child = selected[0]->child(i);
      if ( child->data(1, PRODUCT_ID) == product )
      {
        child->setData(0, Qt::DisplayRole, (extraType == TYPE_WITH) ? "+" : "-");
        child->setData(1, EXTRA_TYPE, extraType);
        child->setHidden(false);

        found = true;
        break;
      }
    }

    if ( !found )
    {
      QSqlQuery query("SELECT name FROM products WHERE id=" + QString::number(product));
      query.next();

      QTreeWidgetItem *child = new QTreeWidgetItem(selected[0]);
      child->setData(0, Qt::DisplayRole, (extraType == TYPE_WITH) ? "+" : "-");
      child->setData(1, EXTRA_TYPE, extraType);
      child->setData(1, Qt::DisplayRole, query.value(0).toString());
      child->setData(1, PRODUCT_ID, product);  // store unique product id

      selected[0]->setExpanded(true);
    }

    // make this a one-time action
    ui.withButton->setChecked(false);
    ui.withoutButton->setChecked(false);
    return;
  }

  // find product in tree and update count, but only if this item does not
  // have already extra child-items defined
  bool found = false;
  for (int i = 0; i < ui.orderList->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *item = ui.orderList->topLevelItem(i);
    if ( (item->childCount() == 0) && (item->data(1, PRODUCT_ID) == product) )
    {
      if ( item->isHidden() )
      {
        item->setData(0, Qt::DisplayRole, 1);
        item->setHidden(false);
      }
      else
        item->setData(0, Qt::DisplayRole, item->data(0, Qt::DisplayRole).toInt() + 1);

      found = true;
      break;
    }
  }

  if ( !found )
  {
    QSqlQuery query("SELECT name, price FROM products WHERE id=" + QString::number(product));
    query.next();

    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setData(0, Qt::DisplayRole, 1);
    item->setData(1, Qt::DisplayRole, query.value(0).toString());
    item->setData(1, PRODUCT_ID, product);  // store unique product id
    item->setData(1, PRODUCT_PRICE, query.value(1).toDouble());
    item->setData(1, ORDER_ID, -1);
    item->setSizeHint(0, QSize(50, 50));
    ui.orderList->addTopLevelItem(item);
    ui.orderList->scrollToBottom();
    ui.orderList->setCurrentItem(item);
  }

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void MainWidget::plusSlot()
{
  QList<QTreeWidgetItem*> selected = ui.orderList->selectedItems();

  if ( selected.isEmpty() || selected[0]->parent() )  // only on toplevel items
    return;

  selected[0]->setData(0, Qt::DisplayRole, selected[0]->data(0, Qt::DisplayRole).toInt() + 1);

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void MainWidget::minusSlot()
{
  QList<QTreeWidgetItem*> selected = ui.orderList->selectedItems();

  if ( selected.isEmpty() || selected[0]->parent() )  // only on toplevel items
    return;

  if ( selected[0]->data(0, Qt::DisplayRole).toInt() > 1 )
    selected[0]->setData(0, Qt::DisplayRole, selected[0]->data(0, Qt::DisplayRole).toInt() - 1);

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void MainWidget::removeSlot()
{
  QList<QTreeWidgetItem*> selected = ui.orderList->selectedItems();

  if ( selected.isEmpty() )
    return;

  selected[0]->setHidden(true);

  if ( selected[0]->parent() == 0 )  // toplevel item (order item) was removed
    updateOrderSum();
}

//--------------------------------------------------------------------------------

void MainWidget::updateOrderSum()
{
  double sum = 0;

  for (int i = 0; i < ui.orderList->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *item = ui.orderList->topLevelItem(i);
    if ( !item->isHidden() )  // shall be removed
      sum += item->data(0, Qt::DisplayRole).toInt() * item->data(1, PRODUCT_PRICE).toDouble();
  }

  ui.sumLabel->setText(QString::number(sum, 'f', 2));
}

//--------------------------------------------------------------------------------

void MainWidget::tableSlot()
{
  // select a table
  QPoint p(ui.tableButton->x() + ui.tableButton->width() - tableSelection->sizeHint().width(),
           ui.tableButton->y() - tableSelection->sizeHint().height());
  int table = tableSelection->exec(mapToGlobal(p));
  if ( table == 0 )
    return;

  currentTable = table;
  ui.tableButton->setText(QString::number(currentTable));
}

//--------------------------------------------------------------------------------
// store values in memory into the DB

bool MainWidget::finishOrder()
{
  if ( ui.orderList->topLevelItemCount() == 0 ) // nothing ordered
    return false;

  if ( currentTicket == 0 )  // a new ticket
  {
    QSqlQuery query;
    query.exec(QString("INSERT INTO tickets (`table`, `timestamp`) VALUES(%1, '%2')")
                       .arg(currentTable)
                       .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

    if ( QSqlDatabase::database().driverName() == "QMYSQL" )
      query.exec("SELECT LAST_INSERT_ID()");
    else if ( QSqlDatabase::database().driverName() == "QSQLITE" )
      query.exec("SELECT last_insert_rowid()");

    query.next();
    currentTicket = query.value(0).toInt();
  }
  else
  {
    // change table in DB
    QSqlQuery query;
    query.exec(QString("UPDATE tickets SET `table`=%1 WHERE id=%2").arg(currentTable).arg(currentTicket));
  }

  for (int i = 0; i < ui.orderList->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *item = ui.orderList->topLevelItem(i);
    int orderId = item->data(1, ORDER_ID).toInt();

    QSqlQuery query;

    if ( item->isHidden() )  // shall be removed
    {
      if ( orderId != -1 )  // was already in DB
      {
        // ordersExtras before orders, as ordersExtras refers to orders.id
        query.exec(QString("DELETE FROM orderExtras WHERE orderId=%1").arg(orderId));
        query.exec(QString("DELETE FROM orders WHERE id=%1").arg(orderId));
      }
    }
    else  // insert or update
    {
      if ( orderId != -1 )  // was already in DB
      {
        QSqlQuery update(QString("UPDATE orders SET count=%1 WHERE id=%2")
                                 .arg(item->data(0, Qt::DisplayRole).toInt())
                                 .arg(orderId));
      }
      else
      {
        query.exec(QString("INSERT INTO orders (ticketId, count, product, price) VALUES(%1, %2, %3, %4)")
                           .arg(currentTicket)
                           .arg(item->data(0, Qt::DisplayRole).toInt())
                           .arg(item->data(1, PRODUCT_ID).toInt())
                           .arg(item->data(1, PRODUCT_PRICE).toDouble()));

        if ( QSqlDatabase::database().driverName() == "QMYSQL" )
          query.exec("SELECT LAST_INSERT_ID()");
        else if ( QSqlDatabase::database().driverName() == "QSQLITE" )
          query.exec("SELECT last_insert_rowid()");

        query.next();
        orderId = query.value(0).toInt();
      }

      // make it simple: delete all orderExtras for this order item and insert all we want now
      query.exec(QString("DELETE FROM orderExtras WHERE orderId=%1").arg(orderId));

      for (int j = 0; j < item->childCount(); j++)
      {
        QTreeWidgetItem *child = item->child(j);
        if ( !child->isHidden() )
        {
          query.exec(QString("INSERT INTO orderExtras (orderId, type, product) VALUES(%1, %2, %3)")
                             .arg(orderId)
                             .arg(child->data(1, EXTRA_TYPE).toInt())
                             .arg(child->data(1, PRODUCT_ID).toInt()));
        }
      }
    }
  }

  ui.openTickets->refreshTickets();

  return true;
}

//--------------------------------------------------------------------------------
// print all unprinted orders which have to be prepared for serving

void MainWidget::printUnprintedOrders(int ticket)
{
  QSqlQuery query(QString("SELECT `table` FROM tickets WHERE id=%1").arg(ticket));
  query.next();
  int table = query.value(0).toInt();

  // print unprinted orders
  QSqlQuery orders(QString("SELECT (orders.count - orders.printed), products.name, orders.id FROM orders "
                           " LEFT JOIN products ON orders.product=products.id"
                           " WHERE orders.ticketId=%1 AND (orders.count > orders.printed)")
                           .arg(ticket));

  if ( orders.next() )
  {
    QPrinter printer;

    if ( initPrinter(printer) )
    {
      QPainter painter(&printer);

      QFont font("Arial", 8);
      painter.setFont(font);
      QFontMetrics fontMetr = painter.fontMetrics();

      const int WIDTH = printer.pageRect().width();

      if ( printer.outputFormat() != QPrinter::PdfFormat )
      {
        painter.translate(WIDTH, printer.pageRect().height());
        painter.rotate(180);
      }

      int y = 0;
      painter.drawText(5, y, WIDTH - 10, fontMetr.height(), Qt::AlignLeft,
                       tr("Datum: %1").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate)));
      y += 5 + fontMetr.height();
      painter.drawText(5, y, WIDTH - 10, fontMetr.height(), Qt::AlignLeft,
                       (table == -1) ? tr("Zum Mitnehmen") : tr("Tisch: %1").arg(table));

      y += 5 + fontMetr.height();
      painter.drawLine(5, y, WIDTH - 5, y);
      y += 5;

      // paint orders
      do
      {
        painter.drawText(5, y, 40, fontMetr.height(), Qt::AlignLeft, QString::number(orders.value(0).toInt()));

        QString product = fontMetr.elidedText(orders.value(1).toString(), Qt::ElideMiddle, WIDTH - 40);
        painter.drawText(40, y, WIDTH - 40, fontMetr.height(), Qt::AlignLeft, product);

        y += 5 + fontMetr.height();
        if ( (y + fontMetr.height()) > printer.pageRect().height() )
        {
          printer.newPage();
          y = 0;
        }

        // check for orderExtras
        QSqlQuery extras(QString("SELECT orderExtras.type, products.name FROM orderExtras "
                                 " LEFT JOIN products ON orderExtras.product=products.id"
                                 " WHERE orderId=%1")
                                 .arg(orders.value(2).toInt()));

        while ( extras.next() )
        {
          painter.drawText(35, y, WIDTH - 35, fontMetr.height(), Qt::AlignLeft,
                           QString((extras.value(0).toInt() == TYPE_WITH) ? "+" : "-") + ' ' +
                           extras.value(1).toString());

          y += 5 + fontMetr.height();
          if ( (y + fontMetr.height()) > printer.pageRect().height() )
          {
            printer.newPage();
            y = 0;
          }
        }
      }
      while ( orders.next() );

      painter.end();
      if ( printer.outputFormat() == QPrinter::PdfFormat )
        QProcess::startDetached("xdg-open print.pdf");

      //orders.exec(QString("UPDATE orders SET printed=count WHERE ticketId=%1").arg(ticket));
      ui.openTickets->refreshTickets();
    }
  }
}

//--------------------------------------------------------------------------------

void MainWidget::donePrintSlot()
{
  doneSlot(true);
}

//--------------------------------------------------------------------------------

void MainWidget::payNowSlot()
{
  if ( !finishOrder() )
  {
    cancelSlot();
    return;
  }

  if ( !finishTicket(currentTicket) )
    return;

  setCurrentIndex(0);
}

//--------------------------------------------------------------------------------

void MainWidget::menuSlot()
{
  QPoint p(ui.menuButton->x() + ui.menuButton->width() - menu->sizeHint().width(),
           ui.menuButton->y() - menu->sizeHint().height());

  menu->move(mapToGlobal(p));
  menu->show();
}

//--------------------------------------------------------------------------------

void MainWidget::exitSlot()
{
  if ( QMessageBox::question(this, tr("Beenden"), tr("Möchten sie wirklich beenden ?"),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
  {
    QApplication::exit();
  }
}

//--------------------------------------------------------------------------------

void MainWidget::settingsSlot()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "LillePOS", "LillePOS");
  SettingsDialog dialog(this, settings);
  if ( dialog.exec() == QDialog::Accepted )
  {
    init();
  }
}

//--------------------------------------------------------------------------------

void MainWidget::doneSlot(bool printOrders)
{
  if ( !finishOrder() )
  {
    cancelSlot();
    return;
  }

  if ( printOrders )
    printUnprintedOrders(currentTicket);

  cancelSlot();
}

//--------------------------------------------------------------------------------

void MainWidget::cancelSlot()
{
  currentTable = 0;
  currentTicket = 0;
  setCurrentIndex(0);
}

//--------------------------------------------------------------------------------

bool MainWidget::initPrinter(QPrinter &printer)
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "LillePOS", "LillePOS");

  if ( noPrinter )
    printer.setOutputFileName("print.pdf");
  else
    printer.setPrinterName(settings.value("ticketPrinter").toString());

  // defaults fit for 2"(width) x 4"(height) settings for the page of a TSC/TTP-245C printer
  printer.setPaperSize(QSizeF(settings.value("paperWidth", 57).toInt(),
                              settings.value("paperHeight", 101).toInt()), QPrinter::Millimeter);

  printer.setPageMargins(settings.value("marginLeft", 0).toInt(),
                         settings.value("marginTop", 17).toInt(),
                         settings.value("marginRight", 5).toInt(),
                         settings.value("marginBottom", 0).toInt(),
                         QPrinter::Millimeter);
  return true;
}

//--------------------------------------------------------------------------------
// show all tickets to be able to change payedBy column

void MainWidget::reopenTicket()
{
  ClosedTickets dialog(this);

  if ( dialog.exec() == ClosedTickets::CHANGE_TICKET )
  {
    int ticket = dialog.getTicket();
    if ( ticket > 0 )
      changeTicket(ticket);
  }
}

//--------------------------------------------------------------------------------

void MainWidget::reportsSlot()
{
  DaySelectorDialog dialog(this);

  if ( dialog.exec() == QDialog::Rejected )
    return;

  QString html;

  if ( dialog.getReportType() == DaySelectorDialog::TURNOVER )
    html = createTurnoverReport(dialog);
  else  // DaySelectorDialog::STATISTICS
    html = createStatisticsReport(dialog);

  QTextDocument doc;
  doc.setHtml(html);

  ReportView viewer(this, &doc);
  viewer.exec();

  if ( dialog.doBackup() )
    doBackup();
}

//--------------------------------------------------------------------------------

QString MainWidget::getTimeQueryString(const DaySelectorDialog &dialog)
{
  QDate date = dialog.getDate();
  QString timeQuery;

  switch ( dialog.getRange() )
  {
    case DaySelectorDialog::DAY:
    {
      timeQuery = QString("and DATE(tickets.timestamp)='%1'").arg(date.toString(Qt::ISODate));
      break;
    }
    case DaySelectorDialog::MONTH:
    {
      QDate from, to;
      from.setDate(date.year(), date.month(), 1);
      to.setDate(date.year(), date.month(), date.daysInMonth());

      timeQuery = QString("and (DATE(tickets.timestamp) >= '%1') and (DATE(tickets.timestamp) <= '%2')")
                          .arg(from.toString(Qt::ISODate))
                          .arg(to.toString(Qt::ISODate));
      break;
    }
    case DaySelectorDialog::YEAR:
    {
      QDate from, to;
      from.setDate(date.year(), 1, 1);
      to.setDate(date.year(), 12, 31);

      timeQuery = QString("and (DATE(tickets.timestamp) >= '%1') and (DATE(tickets.timestamp) <= '%2')")
                          .arg(from.toString(Qt::ISODate))
                          .arg(to.toString(Qt::ISODate));
      break;
    }
    default:  // all data
    {
      // no restriction
    }
  }

  return timeQuery;
}

//--------------------------------------------------------------------------------

QString MainWidget::createTurnoverReport(const DaySelectorDialog &dialog)
{
  QDate date = dialog.getDate();

  QString html;

  switch ( dialog.getRange() )
  {
    case DaySelectorDialog::DAY:
    {
      html = QString("<html><h1>%1 %2</h1>")
                     .arg(tr("Tagesbericht vom"))
                     .arg(date.toString(Qt::SystemLocaleLongDate));
      break;
    }
    case DaySelectorDialog::MONTH:
    {
      html = QString("<html><h1>%1 %2</h1>")
                     .arg(tr("Monatsbericht"))
                     .arg(date.toString("MMMM yyyy"));
      break;
    }
    case DaySelectorDialog::YEAR:
    {
      html = QString("<html><h1>%1 %2</h1>")
                     .arg(tr("Jahresbericht"))
                     .arg(date.toString("yyyy"));
      break;
    }
    case DaySelectorDialog::ALL:
    {
      html = QString("<html><h1>%1</h1>")
                     .arg(tr("Bericht: Alle Daten"));
      break;
    }
  }

  const int payedByArray[] = { PAYED_BY_CASH, PAYED_BY_ADVERTISING, PAYED_BY_PRIVATE, PAYED_BY_EMPLOYEE };

  for (int i = 0; i < static_cast<int>(sizeof(payedByArray) / sizeof(int)); i++)
  {
    double sum = 0.0;
    int payedBy = payedByArray[i];

    if ( payedBy != PAYED_BY_CASH )
      html += QString("<hr> <h2>%1</h2>").arg(getPayedByText(payedBy));

    html += "<table border='1'>";
    html += QString("<thead> <tr> <th>%1</th> <th>%2</th> <th>%3</th> </tr> </thead> <tbody>")
                    .arg(tr("Anzahl"))
                    .arg(tr("Gruppen"))
                    .arg(tr("Summen"));

    QString timeQuery = getTimeQueryString(dialog);
    QString ticketsFilter;

    if ( payedBy == PAYED_BY_CASH )
      ticketsFilter = "tickets.payedBy >= 0";   // cash or ATM
    else
      ticketsFilter = QString("tickets.payedBy = %1").arg(payedBy);

    // price-sum over all groups and sum over products in these groups
    QSqlQuery query(QString("SELECT SUM(orders.count), groups.name, SUM(orders.count * orders.price) from tickets"
                            " LEFT JOIN orders on orders.ticketId=tickets.id"
                            " LEFT JOIN products on orders.product=products.id"
                            " LEFT JOIN groups on `products`.`group`=`groups`.`id`"
                            " WHERE tickets.open=0 AND " + ticketsFilter + " " + timeQuery +
                            " GROUP by groups.name"));

    while ( query.next() )
    {
      html += "<tr>";
      html += "<td align='right' style='padding-right: 10px; padding-left: 10px;'>" +  QString::number(query.value(0).toInt()) + "</td>";
      html += "<td style='padding-right: 10px; padding-left: 10px;'>" + query.value(1).toString() + "</td>";
      html += "<td align='right'>" + QString::number(query.value(2).toDouble(), 'f', 2) + "</td>";
      html += "</tr>";

      sum += query.value(2).toDouble();
    }

    html += QString("<tr> <td colspan='2' align='right'><b>%1</b></td> <td align='right'><b>%2</b></td> </tr>")
                    .arg(tr("Gesamt:"))
                    .arg(QString::number(sum, 'f', 2));

    // sum of different tax rates; tax = (brutto * tax/100) / (1 + tax/100)
    query.exec(QString("SELECT products.tax, SUM((orders.count * orders.price * products.tax/100.0) / (1.0 + (products.tax/100.0))) from tickets"
                       " LEFT JOIN orders on orders.ticketId=tickets.id"
                       " LEFT JOIN products on orders.product=products.id"
                       " LEFT JOIN groups on `products`.`group`=`groups`.`id`"
                       " WHERE tickets.open=0 AND " + ticketsFilter + " " + timeQuery +
                       " GROUP by products.tax"));

    while ( query.next() )
    {
      html += QString("<tr> <td colspan='2' align='right'><nobr><b>%1</b></nobr></td> <td align='right'><b>%2</b></td> </tr>")
                      .arg(tr("Summe %1 % MwSt:").arg(query.value(0).toInt()))
                      .arg(QString::number(query.value(1).toDouble(), 'f', 2));
    }

    if ( payedBy == PAYED_BY_CASH )  // show cash/ATM separately
    {
      query.exec(QString("SELECT SUM(orders.count * orders.price), tickets.payedBy from orders"
                         " LEFT JOIN tickets on orders.ticketId=tickets.id"
                         " WHERE tickets.open=0 AND " + ticketsFilter + " " + timeQuery +
                         " GROUP BY tickets.payedBy"));

      while ( query.next() )
      {
        html += QString("<tr> <td colspan='2' align='right'><b>%1</b></td> <td align='right'><b>%2</b></td> </tr>")
                        .arg(tr("Summe %1:").arg(getPayedByText(query.value(1).toInt())))
                        .arg(QString::number(query.value(0).toDouble(), 'f', 2));
      }
    }

    html += "</tbody> </table>";
  }

  html += "</html>";

  return html;
}

//--------------------------------------------------------------------------------

QString MainWidget::createStatisticsReport(const DaySelectorDialog &dialog)
{
  QDate date = dialog.getDate();

  QString html;

  switch ( dialog.getRange() )
  {
    case DaySelectorDialog::DAY:
    {
      html = QString("<html><h1>%1 %2</h1>")
                     .arg(tr("Statistik"))
                     .arg(date.toString(Qt::SystemLocaleLongDate));
      break;
    }
    case DaySelectorDialog::MONTH:
    {
      html = QString("<html><h1>%1 %2</h1>")
                     .arg(tr("Statistik"))
                     .arg(date.toString("MMMM yyyy"));
      break;
    }
    case DaySelectorDialog::YEAR:
    {
      html = QString("<html><h1>%1 %2</h1>")
                     .arg(tr("Statistik"))
                     .arg(date.toString("yyyy"));
      break;
    }
    case DaySelectorDialog::ALL:
    {
      html = QString("<html><h1>%1</h1>")
                     .arg(tr("Statistik: Alle Daten"));
      break;
    }
  }

  QString timeQuery = getTimeQueryString(dialog);

  // separate tables per group
  QSqlQuery groupsQuery("SELECT id,name FROM groups ORDER BY name");

  while ( groupsQuery.next() )
  {
    QString filter = QString(" AND products.`group`=%1 ").arg(groupsQuery.value(0).toInt());
    html += QString("<h3>%1</h3>").arg(groupsQuery.value(1).toString());

    html += "<table border='1'>";
    html += QString("<tr> <th>%1</th> <th>%2</th> <th>%3</th> </tr>")
                    .arg(tr("Anzahl"))
                    .arg(tr("Produkt"))
                    .arg(tr("Summe"));

    QSqlQuery query(QString("SELECT SUM(orders.count), products.name, SUM(orders.count * orders.price)"
                            " FROM tickets"
                            " LEFT JOIN orders on orders.ticketId=tickets.id"
                            " LEFT JOIN products on orders.product=products.id"
                            " WHERE tickets.invoiceNum is not null " + filter + timeQuery + " GROUP by products.name"));

    double sum = 0.0;

    // calc max count
    int maxCount = 0;
    while ( query.next() )
      maxCount = qMax(maxCount, query.value(0).toInt());

    if ( maxCount > 0 )
    {
      query.first(); // back to first record
      do
      {
        html += "<tr>";
        html += "<td align='right'>" +  QString::number(query.value(0).toInt()) + "</td>";
        html += "<td style='padding-right: 5px; padding-left: 5px;'>" + query.value(1).toString() + "</td>";
        html += "<td align='right'>" + QString::number(query.value(2).toDouble(), 'f', 2) + "</td>";

        // show a "bar graph" of the relative value
        html += "<td>" + QString("|" /* "&#10072;" */).repeated((query.value(0).toInt() * 100) / maxCount) + "</td>";

        html += "</tr>";

        sum += query.value(2).toDouble();
      }
      while ( query.next() );
    }

    html += QString("<tr> <td colspan='2' align='right'><b>%1</b></td> <td align='right'><b>%2</b></td> </tr>")
                    .arg(tr("Gesamt:"))
                    .arg(QString::number(sum, 'f', 2));

    html += "</table>";
  }

  html += "</html>";

  return html;
}

//--------------------------------------------------------------------------------

void MainWidget::doBackup()
{
  // copy to target (e.g. ftp server, local file, ...)
  QUrl target;

  {
    QSqlQuery query("SELECT strValue FROM globals WHERE name='backupTarget'");
    if ( query.next() && !query.value(0).toString().isEmpty() )
      target = query.value(0).toUrl();
  }

  if ( !target.isValid() )  // no backup target defined
    return;

  QString sourceFileName, targetFileName;
  QTemporaryFile tmpFile;

  if ( QSqlDatabase::database().driverName() == "QSQLITE" )
  {
    // backup the db-file
    sourceFileName = targetFileName = QSqlDatabase::database().databaseName();
  }
  else if ( QSqlDatabase::database().driverName() == "QMYSQL" )
  {
    // create a temporary dump file which will be backed up
    tmpFile.open();
    sourceFileName = tmpFile.fileName();
    targetFileName = QLatin1String("LillePOS.sql");

    QProcess proc;
    proc.setStandardOutputFile(sourceFileName);
    proc.start(QString("mysqldump LillePOS --user=%1 --password=%2")
                       .arg(QSqlDatabase::database().userName())
                       .arg(QSqlDatabase::database().password()));
    if ( !proc.waitForFinished(10000) )
      return;
  }
  else
    return;

  target.setPath(target.path() + QLatin1Char('/') + targetFileName);

  QSqlDatabase::database().close();
  QString connectionName = QSqlDatabase::database().connectionName();
  QSqlDatabase::removeDatabase(connectionName);

  Backup dialog(this, sourceFileName, target);
  dialog.exec();

  openDB(false);
}

//--------------------------------------------------------------------------------

void MainWidget::pictureSelected(int num)
{
  int row, column;
  row = num / PIC_BUTTONS_PER_ROW;
  column = num % PIC_BUTTONS_PER_ROW;

  QLayoutItem *item = ui.pictureButtonsLayout->itemAtPosition(row, column);
  if ( !item )  // just be sure
    return;

  QPushButton *b = qobject_cast<QPushButton*>(item->widget());

  if ( !b )
    return;

  if ( b->isChecked() )
  {
    numPicturesShown++;
    picturesFrame->layout()->itemAt(num)->widget()->show();
  }
  else
  {
    numPicturesShown--;
    picturesFrame->layout()->itemAt(num)->widget()->hide();
  }

  picturesFrame->setVisible(numPicturesShown > 0);
  picturesFrame->setGeometry(ui.openTickets->geometry());  // overlaps completely
  picturesFrame->raise();

  pictureFrameArrow->setIcon(QIcon(":/icons/arrow-right.png"));
  pictureFrameArrow->setVisible(numPicturesShown > 0);
  pictureFrameArrow->move(width() - pictureFrameArrow->width(), ui.openTickets->y());
  pictureFrameArrow->raise(); // top most
}

//--------------------------------------------------------------------------------

void MainWidget::printInvoice(int ticket)
{
  // print ticket
  QPrinter printer;

  if ( initPrinter(printer) )
  {
    QSqlQuery query(QString("SELECT `table`,`invoiceNum`,`timestamp` FROM tickets WHERE id=%1").arg(ticket));
    query.next();
    int table = query.value(0).toInt();
    int invoiceNum = query.value(1).toInt();
    QDateTime invoiceTime = query.value(2).toDateTime();

    QPainter painter(&printer);
    /*
    painter.drawLine(0, 0, 100, 0);
    painter.drawLine(0, 0, printer.pageRect().width(), printer.pageRect().height());
    printer.newPage();
    painter.drawLine(0, 0, 200, 0);
    painter.drawLine(0, 0, printer.pageRect().width()-1, printer.pageRect().height()-1);
    painter.drawLine(printer.pageRect().width()-1, printer.pageRect().height()-1,
                     printer.pageRect().width()-1 - 200, printer.pageRect().height()-1);
    */

    //QSizeF s = printer.paperSize(QPrinter::DevicePixel);
    //qWarning("paper w:%f h:%f", s.width(), s.height());

    QFont font("Arial", 7);
    painter.setFont(font);
    QFontMetrics fontMetr = painter.fontMetrics();

    QFont priceFont(font);
    priceFont.setFixedPitch(true);
    QFontMetrics priceMetrics(priceFont, &printer);

    const int WIDTH = printer.pageRect().width();

    if ( printer.outputFormat() != QPrinter::PdfFormat )
    {
      painter.translate(WIDTH, printer.pageRect().height());
      painter.rotate(180);  // for tests we use PDF but we don't want to see the printout upside down
    }

    int y = 0;

    QPixmap logoPixmap;
    logoPixmap.load(":/logo.png");

    painter.drawPixmap(WIDTH - logoPixmap.width() - 1, y, logoPixmap);

    QRect rect = painter.boundingRect(0, y, WIDTH - logoPixmap.width(), logoPixmap.height(), Qt::AlignCenter, shopName);
    painter.drawText(0, y, rect.width(), rect.height(), Qt::AlignCenter, shopName);

    y += 5 + qMax(rect.height(), logoPixmap.height()) + 4;
    painter.drawLine(0, y, WIDTH, y);

    y += 5;
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Datum: %1").arg(invoiceTime.toString(Qt::SystemLocaleShortDate)));
    y += 5 + fontMetr.height();
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Rechnung: %1  -  %2")
                        .arg(invoiceNum)
                        .arg((table == -1) ? tr("Zum Mitnehmen") : tr("Tisch: %1").arg(table)));

    y += 5 + fontMetr.height();
    painter.drawLine(0, y, WIDTH, y);

    // paint orders
    QSqlQuery orders(QString("SELECT orders.count, products.name, orders.price, products.tax FROM orders "
                             " LEFT JOIN products ON orders.product=products.id"
                             " WHERE orders.ticketId=%1")
                             .arg(ticket));

    y += 5;

    const int X_COUNT = 0;
    const int X_NAME  = 25;

    painter.drawText(X_COUNT, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Anz"));
    painter.drawText(X_NAME,  y, WIDTH - X_COUNT,  fontMetr.height(), Qt::AlignLeft, tr("Produkt"));
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, tr("Preis"));
    y += 5 + fontMetr.height();

    double sum = 0.0;
    QMap<int, double> taxes; // <tax-percent, sum>

    while ( orders.next() )
    {
      int count = orders.value(0).toInt();
      double singlePrice = orders.value(2).toDouble();
      double price = singlePrice * count;
      int tax = orders.value(3).toInt();

      sum += price;

      if ( taxes.contains(tax) )
        taxes[tax] += (price * (tax / 100.0)) / (1.0 + tax / 100.0);
      else
        taxes[tax] = (price * (tax / 100.0)) / (1.0 + tax / 100.0);

      painter.drawText(X_COUNT, y, WIDTH - X_COUNT, fontMetr.height(), Qt::AlignLeft, QString::number(count));

      QString priceText = QString::number(price, 'f', 2);
      if ( count > 1 )
        priceText = QString("(%1) %2").arg(QString::number(singlePrice, 'f', 2)).arg(priceText);
      int priceWidth = priceMetrics.boundingRect(priceText).width();

      QString product = fontMetr.elidedText(orders.value(1).toString(), Qt::ElideMiddle, WIDTH - X_NAME - priceWidth - 5);
      painter.drawText(X_NAME,  y, WIDTH - X_NAME - priceWidth - 5,  fontMetr.height(), Qt::AlignLeft, product);

      painter.setFont(priceFont);
      painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, priceText);
      painter.setFont(font);

      y += 5 + fontMetr.height();
      if ( (y + fontMetr.height()) > printer.pageRect().height() )
      {
        printer.newPage();
        y = 0;
      }
    }

    painter.drawLine(0, y, WIDTH, y);

    QList<int> keys = taxes.keys();

    QFont boldFont("Arial", 12, 100);  // for sum
    QFontMetrics boldMetr(boldFont);

    // if there is not enough space for sum+tax lines, start new page
    if ( (y + (keys.count() * (5 + fontMetr.height())) + boldMetr.height() + 10) > printer.pageRect().height() )
    {
      printer.newPage();
      y = 0;
    }

    y += 5;
    painter.save();
    painter.setFont(boldFont);
    painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignRight,
                     tr("Summe Gesamt: %1").arg(QString::number(sum, 'f', 2)));
    painter.restore();
    y += boldMetr.height() + 10;

    for (int i = 0; i < keys.count(); i++)
    {
      painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight,
                       tr("Summe MwSt %1%: %2").arg(keys[i]).arg(QString::number(taxes[keys[i]], 'f', 2)));
      y += 5 + fontMetr.height();
    }

    painter.end();
    if ( printer.outputFormat() == QPrinter::PdfFormat )
      QProcess::startDetached("xdg-open print.pdf");
  }
}

//--------------------------------------------------------------------------------

QString MainWidget::getPayedByText(int payedBy)
{
  switch ( payedBy )
  {
    case PAYED_BY_EMPLOYEE   : return tr("Mitarbeiter");
    case PAYED_BY_ADVERTISING: return tr("Werbung");
    case PAYED_BY_PRIVATE    : return tr("Privat");
    case PAYED_BY_CASH       : return tr("Bar");
    case PAYED_BY_ATM        : return tr("Bankomat");
  }
  return QString();
}

//--------------------------------------------------------------------------------

void MainWidget::workingTimesSlot()
{
  WorkingTimes dialog(this);
  dialog.exec();
}

//--------------------------------------------------------------------------------

void MainWidget::fullScreenSlot()
{
  menu->hide();

  if ( isFullScreen() )
  {
    showNormal();
    // WeTab does not reparent/show with window frame if not resized
    resize(width() - 150, height());
    showMaximized();
  }
  else
    showFullScreen();
}

//--------------------------------------------------------------------------------

void MainWidget::resizeEvent(QResizeEvent *event)
{
  picturesFrame->setGeometry(ui.openTickets->geometry());  // overlaps completely
  pictureFrameArrow->move(width() - pictureFrameArrow->width(), ui.openTickets->y());

  QStackedWidget::resizeEvent(event);
}

//--------------------------------------------------------------------------------

void MainWidget::movePicturesSlot()
{
  if ( picturesFrame->x() == ui.openTickets->x() )
  {
#if QT_VERSION >= 0x040600
    pictureAnimation->setStartValue(QPoint(picturesFrame->x(), picturesFrame->y()));
    pictureAnimation->setEndValue(QPoint(width(), picturesFrame->y()));
#else
    picturesFrame->move(QPoint(width(), picturesFrame->y()));
#endif
    pictureFrameArrow->setIcon(QIcon(":/icons/arrow-left.png"));
  }
  else
  {
#if QT_VERSION >= 0x040600
    pictureAnimation->setStartValue(QPoint(picturesFrame->x(), picturesFrame->y()));
    pictureAnimation->setEndValue(QPoint(ui.openTickets->x(), picturesFrame->y()));
#else
    picturesFrame->move(QPoint(ui.openTickets->x(), picturesFrame->y()));
#endif
    pictureFrameArrow->setIcon(QIcon(":/icons/arrow-right.png"));
  }
#if QT_VERSION >= 0x040600
  pictureAnimation->start();
#endif
}

//--------------------------------------------------------------------------------
// only one of "with" our "without" may be checked

void MainWidget::withButtonSlot()
{
  if ( ui.withoutButton->isChecked() )
    ui.withoutButton->setChecked(false);
}

//--------------------------------------------------------------------------------
// only one of "with" our "without" may be checked

void MainWidget::withoutButtonSlot()
{
  if ( ui.withButton->isChecked() )
    ui.withButton->setChecked(false);
}

//--------------------------------------------------------------------------------

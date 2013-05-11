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

#include <SettingsDialog.hxx>
#include <CategoriesWidget.hxx>
#include <GroupsWidget.hxx>
#include <ProductsWidget.hxx>
#include <PictureButtonSettings.hxx>
#include <EmployeesWidget.hxx>

#include <QSqlQuery>
#include <QPrinterInfo>

//--------------------------------------------------------------------------------

SettingsDialog::SettingsDialog(QWidget *parent, QSettings &s)
  : QDialog(parent), settings(s)
{
  ui.setupUi(this);

  connect(ui.categoriesButton, SIGNAL(clicked()), this, SLOT(categoriesSlot()));
  connect(ui.groupsButton,     SIGNAL(clicked()), this, SLOT(groupsSlot()));
  connect(ui.productsButton,   SIGNAL(clicked()), this, SLOT(productsSlot()));
  connect(ui.picturesButton,   SIGNAL(clicked()), this, SLOT(picturesSlot()));
  connect(ui.employeesButton,  SIGNAL(clicked()), this, SLOT(employeesSlot()));

  QSqlQuery query("SELECT value FROM globals WHERE name='numTables'");
  query.next();
  ui.numTables->setValue(query.value(0).toInt());

  query.exec("SELECT strValue FROM globals WHERE name='shopName'");
  query.next();
  ui.printHeader->setText(query.value(0).toString());

  QString ticketPrinter = settings.value("ticketPrinter").toString();
  QString reportPrinter = settings.value("reportPrinter").toString();
  QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
  for (int i = 0; i < availablePrinters.count(); i++)
  {
    ui.ticketPrinter->addItem(availablePrinters[i].printerName());
    ui.reportPrinter->addItem(availablePrinters[i].printerName());
    if ( ticketPrinter == availablePrinters[i].printerName() )
      ui.ticketPrinter->setCurrentIndex(i);
    if ( reportPrinter == availablePrinters[i].printerName() )
      ui.reportPrinter->setCurrentIndex(i);
  }

  ui.paperWidth->setValue(settings.value("paperWidth", 57).toInt());
  ui.paperHeight->setValue(settings.value("paperHeight", 101).toInt());

  ui.marginLeft->setValue(settings.value("marginLeft", 0).toInt());
  ui.marginTop->setValue(settings.value("marginTop", 17).toInt());
  ui.marginRight->setValue(settings.value("marginRight", 5).toInt());
  ui.marginBottom->setValue(settings.value("marginBottom", 0).toInt());

  query.exec("SELECT strValue FROM globals WHERE name='backupTarget'");
  if ( query.next() )
    ui.backupTarget->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('backupTarget', '')");
}

//--------------------------------------------------------------------------------

void SettingsDialog::accept()
{
  QSqlQuery query;
  query.exec(QString("UPDATE globals SET value=%1 WHERE name='numTables'").arg(ui.numTables->value()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopName'").arg(ui.printHeader->toPlainText()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='backupTarget'").arg(ui.backupTarget->text()));

  settings.setValue("reportPrinter", ui.reportPrinter->currentText());
  settings.setValue("ticketPrinter", ui.ticketPrinter->currentText());
  settings.setValue("paperWidth", ui.paperWidth->value());
  settings.setValue("paperHeight", ui.paperHeight->value());
  settings.setValue("marginLeft", ui.marginLeft->value());
  settings.setValue("marginTop", ui.marginTop->value());
  settings.setValue("marginRight", ui.marginRight->value());
  settings.setValue("marginBottom", ui.marginBottom->value());

  QDialog::accept();
}

//--------------------------------------------------------------------------------

void SettingsDialog::categoriesSlot()
{
  CategoriesWidget *cat = new CategoriesWidget(this);
  cat->setAttribute(Qt::WA_DeleteOnClose);
  cat->setModal(true);
  cat->show();
}

//--------------------------------------------------------------------------------

void SettingsDialog::groupsSlot()
{
  GroupsWidget *grp = new GroupsWidget(this);
  grp->setAttribute(Qt::WA_DeleteOnClose);
  grp->setModal(true);
  grp->show();
}

//--------------------------------------------------------------------------------

void SettingsDialog::productsSlot()
{
  ProductsWidget *prod = new ProductsWidget(this);
  prod->setAttribute(Qt::WA_DeleteOnClose);
  prod->setModal(true);
  prod->show();
}

//--------------------------------------------------------------------------------

void SettingsDialog::picturesSlot()
{
  PictureButtonSettings *dialog = new PictureButtonSettings(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setModal(true);
  dialog->show();
}

//--------------------------------------------------------------------------------

void SettingsDialog::employeesSlot()
{
  EmployeesWidget *dialog = new EmployeesWidget(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setModal(true);
  dialog->show();
}

//--------------------------------------------------------------------------------

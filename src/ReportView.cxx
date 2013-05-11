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

#include <ReportView.hxx>

#include <QPrinter>
#include <QSettings>
#include <QPrintDialog>

//--------------------------------------------------------------------------------

ReportView::ReportView(QWidget *parent, QTextDocument *doc)
  : QDialog(parent), document(doc)
{
  ui.setupUi(this);
  ui.textBrowser->setDocument(document);

  connect(ui.print, SIGNAL(clicked()), this, SLOT(printSlot()));
}

//--------------------------------------------------------------------------------

void ReportView::printSlot()
{
  QPrinter printer;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "LillePOS", "LillePOS");
  printer.setPrinterName(settings.value("reportPrinter").toString());

  QPrintDialog dialog(&printer, this);
  if ( dialog.exec() == QDialog::Rejected )
    return;

  document->print(&printer);
}

//--------------------------------------------------------------------------------

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

#include <FinishTicketDialog.hxx>

#include <QStyle>

//--------------------------------------------------------------------------------

FinishTicketDialog::FinishTicketDialog(QWidget *parent, bool invoiceExists, double sum)
  : QDialog(parent), origSum(sum)
{
  ui.setupUi(this);

  int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
  QIcon tmpIcon(style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, this));
  ui.icon->setPixmap(tmpIcon.pixmap(iconSize, iconSize));

  showSum(origSum);

  connect(ui.withPrintout, SIGNAL(clicked()), this, SLOT(withPrintout()));
  connect(ui.withoutPrintout, SIGNAL(clicked()), this, SLOT(withoutPrintout()));
  connect(ui.privateTicket, SIGNAL(clicked()), this, SLOT(privateTicket()));
  connect(ui.employeeTicket, SIGNAL(clicked()), this, SLOT(employeeTicket()));
  connect(ui.advertisingTicket, SIGNAL(clicked()), this, SLOT(advertisingTicket()));

  connect(ui.discount_20, SIGNAL(toggled(bool)), this, SLOT(discountToggled(bool)));

  // if there is already an invoiceNum for this ticket, disallow to change
  // the type of the ticket to one without invoiceNum (private, advertising)
  if ( invoiceExists )
  {
    ui.privateTicket->setEnabled(false);
    ui.employeeTicket->setEnabled(false);
    ui.advertisingTicket->setEnabled(false);
  }
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::withPrintout()
{
  done(WITH_PRINTOUT);
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::withoutPrintout()
{
  done(WITHOUT_PRINTOUT);
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::privateTicket()
{
  done(PRIVATE_TICKET);
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::employeeTicket()
{
  done(EMPLOYEE_TICKET);
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::advertisingTicket()
{
  done(ADVERTISING_TICKET);
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::discountToggled(bool on)
{
  if ( on )
    showSum(origSum * 0.8);
  else
    showSum(origSum);
}

//--------------------------------------------------------------------------------

void FinishTicketDialog::showSum(double sum)
{
  ui.label->setText(tr("<html>Bon abschlie&szlig;en ?<br>Nicht vergessen <font style='color:red;'>%1</font> zu kassieren!</html>")
                      .arg(QString::number(sum, 'f', 2)));
}

//--------------------------------------------------------------------------------

int FinishTicketDialog::getDiscount() const
{
  if ( ui.discount_20->isChecked() )
    return 20;

  return 0;
}

//--------------------------------------------------------------------------------

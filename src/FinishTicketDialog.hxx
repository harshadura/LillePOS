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

#ifndef _FinishTicketDialog_H_
#define _FinishTicketDialog_H_

#include <QDialog>

#include <ui_FinishTicketDialog.h>

class FinishTicketDialog : public QDialog
{
  Q_OBJECT

  public:
    FinishTicketDialog(QWidget *parent, bool invoiceExists, double sum);

    enum  // exec return codes
    {
      WITH_PRINTOUT = 10,
      WITHOUT_PRINTOUT,
      PRIVATE_TICKET,
      EMPLOYEE_TICKET,
      ADVERTISING_TICKET
    };

    int getDiscount() const;

  private slots:
    void withPrintout();
    void withoutPrintout();
    void privateTicket();
    void employeeTicket();
    void advertisingTicket();
    void discountToggled(bool on);

  private:
    void showSum(double sum);

  private:
    Ui::FinishTicketDialog ui;
    double origSum;
};

#endif

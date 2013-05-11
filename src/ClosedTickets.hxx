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

#ifndef _ClosedTickets_H_
#define _ClosedTickets_H_

#include <QDialog>
class QSqlQueryModel;
class MainWidget;

#include <ui_ClosedTickets.h>

class ClosedTickets : public QDialog
{
  Q_OBJECT

  public:
    enum
    {
      CHANGE_TICKET = 10  // returncode of dialog if "change ticket" was pressed
    };

    ClosedTickets(MainWidget *parent);

    // return id of selected ticket
    int getTicket() const;

  private slots:
    void itemClicked(QTreeWidgetItem * item, int column);
    void printInvoice();
    void changeTicket();
    void searchInvoice(const QString &text);
    void showPrivate(bool priv);
    void fillTree();

  private:
    Ui::ClosedTickets ui;
    QSqlQueryModel *model;
    QString queryString;
};

#endif

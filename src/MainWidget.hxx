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

#ifndef _MAIN_WIDGET_H_
#define _MAIN_WIDGET_H_

#include <QStackedWidget>
#include <QSignalMapper>
class QVBoxLayout;
class QGridLayout;
class QPrinter;
class QFrame;
class QPropertyAnimation;

class TableSelection;
class DaySelectorDialog;

#include <ui_LillePOS.h>

// values for the tickets.payedBy field
enum
{
  PAYED_BY_EMPLOYEE = -3,     // employee consumation
  PAYED_BY_ADVERTISING = -2,  // does not have invoiceNum; was a gift
  PAYED_BY_PRIVATE = -1,      // private consumation
  PAYED_BY_CASH = 0,
  PAYED_BY_ATM = 1
};

//--------------------------------------------------------------------------------

class MainWidget : public QStackedWidget
{
  Q_OBJECT

  public:
    MainWidget();
    bool openDB(bool dbSelect);

    void printInvoice(int ticket);

    void setNoPrinter() { noPrinter = true; }  // do not print to printer but only to pdf file for testing only

    static QString getPayedByText(int payedBy);

  protected:
    void resizeEvent(QResizeEvent *event);

  private slots:
    void newTicket();
    void changeTicket(int ticket = 0);
    void voidTicket();
    void payTicket();
    void payGroupTicket(QList<int> tickets);
    void reopenTicket();

    void plusSlot();
    void minusSlot();
    void removeSlot();
    void tableSlot();
    void withButtonSlot();
    void withoutButtonSlot();
    void donePrintSlot();
    void payNowSlot();
    void doneSlot(bool printOrders = false);
    void cancelSlot();

    void menuSlot();
    void exitSlot();
    void settingsSlot();
    void reportsSlot();
    void workingTimesSlot();
    void fullScreenSlot();

    void categorySelected(int category);
    void groupSelected(int group);
    void productSelected(int product);
    bool initPrinter(QPrinter &printer);  // return false when user cancelled printer selection

    void pictureSelected(int num);
    void movePicturesSlot();

  private:  // methods
    void init();
    void clearGroupsLayout();
    void getSelectedTicket(int &id, int &table);
    bool finishOrder();
    void printUnprintedOrders(int ticket);
    void updateOrderSum();
    bool finishTicket(int ticket);
    void doBackup();
    QString getTimeQueryString(const DaySelectorDialog &dialog);
    QString createTurnoverReport(const DaySelectorDialog &dialog);
    QString createStatisticsReport(const DaySelectorDialog &dialog);

  private:
    Ui::StackedWidget ui;
    QVBoxLayout *categoriesLayout;
    QGridLayout *groupsLayout;
    QSignalMapper categoriesMapper;
    QSignalMapper groupsMapper;
    QSignalMapper productsMapper;
    QFrame *menu;
    TableSelection *tableSelection;

    QFrame *picturesFrame;
    QPushButton *pictureFrameArrow;
    QSignalMapper picturesMapper;
    int numPicturesShown;
    QPropertyAnimation *pictureAnimation;

    int numTables;
    int currentTable;
    int currentTicket;

    QString shopName;

    bool noPrinter;
};

#endif

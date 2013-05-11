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

#ifndef _OPEN_TICKET_WIDGET_H_
#define _OPEN_TICKET_WIDGET_H_

#include <QFrame>
#include <QTreeWidget>

class OpenTicketWidget : public QFrame
{
  Q_OBJECT

  public:
    OpenTicketWidget(QWidget *parent, int theId, int theTable);

    bool isSelected() const { return selected; }
    void setSelected(bool sel = true);

    int getId() const { return id; }
    int getTable() const { return table; }

  private slots:
    void selectedSlot();
    void ordersServed();

  private:
    int id;
    int table;
    bool selected;
    QWidget *toServeWidget;
};

//--------------------------------------------------------------------------------
// helper to allow selection on clicking anywhere in tree

class TreeWidget : public QTreeWidget
{
  Q_OBJECT

  public:
    TreeWidget(QWidget *parent);

  signals:
    void selected();

  protected:
    virtual bool viewportEvent(QEvent *event);
};

#endif

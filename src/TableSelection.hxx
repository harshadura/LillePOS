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

#ifndef _TABLE_SELECTION_H_
#define _TABLE_SELECTION_H_

#include <QFrame>
#include <QEventLoop>
#include <QSignalMapper>

// a popup widget for the selection of a table

class TableSelection : public QFrame
{
  Q_OBJECT

  public:
    TableSelection(QWidget *parent, int numTables);

    // start local event loop and return selected table
    int exec(const QPoint &pos);

  protected:
    virtual void hideEvent(QHideEvent *event);

  private slots:
    void tableSelected(int table);

  private:
    QEventLoop eventLoop;
    QSignalMapper tableSelectionMapper;
};

#endif

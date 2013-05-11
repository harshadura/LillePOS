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

#include <TableSelection.hxx>

#include <QHBoxLayout>
#include <QPushButton>

//--------------------------------------------------------------------------------

TableSelection::TableSelection(QWidget *parent, int numTables)
  : QFrame(parent, Qt::Popup)
{
  QFont font("Arial", 32, 100);

  setFrameStyle(QFrame::StyledPanel);
  QHBoxLayout *hbox = new QHBoxLayout(this);

  QPushButton *b;

  // "to go"
  b = new QPushButton(tr("Mitnehmen"));
  b->setFixedHeight(60);
  b->setFont(font);  // when using stylesheet, the properties are not propagated from the parent
  connect(b, SIGNAL(clicked()), &tableSelectionMapper, SLOT(map()));
  hbox->addWidget(b);
  tableSelectionMapper.setMapping(b, -1);  // -1 .. to go

  // tables
  for (int i = 1; i <= numTables; i++)
  {
    b = new QPushButton(QString::number(i));
    b->setFixedHeight(60);
    b->setFont(font);  // when using stylesheet, the properties are not propagated from the parent
    connect(b, SIGNAL(clicked()), &tableSelectionMapper, SLOT(map()));
    hbox->addWidget(b);
    tableSelectionMapper.setMapping(b, i);
  }

  connect(&tableSelectionMapper, SIGNAL(mapped(int)), this, SLOT(tableSelected(int)));
}

//--------------------------------------------------------------------------------

void TableSelection::hideEvent(QHideEvent *)
{
  if ( eventLoop.isRunning() )
    eventLoop.exit(0);
}

//--------------------------------------------------------------------------------

void TableSelection::tableSelected(int table)
{
  eventLoop.exit(table);
}

//--------------------------------------------------------------------------------

int TableSelection::exec(const QPoint &pos)
{
  move(pos);
  show();
  int ret = eventLoop.exec();
  hide();
  return ret;
}

//--------------------------------------------------------------------------------

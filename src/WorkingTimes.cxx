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

#include <WorkingTimes.hxx>
#include <WorkingTimesList.hxx>
#include <MonthSelectorDialog.hxx>
#include <ReportView.hxx>
#include <DateTimeEdit.hxx>

#include <QSqlQuery>
#include <QPushButton>
#include <QDateTime>
#include <QSqlTableModel>
#include <QTableView>
#include <QDialog>
#include <QTextDocument>
#include <QTextBrowser>

#define MIN_HEIGHT 60

//--------------------------------------------------------------------------------

WorkingTimes::WorkingTimes(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);

  QSqlQuery query("SELECT id, name, start from employees");

  while ( query.next() )
  {
    QHBoxLayout *hbox = new QHBoxLayout;

    QPushButton *b = new QPushButton;
    b->setCheckable(true);
    b->setFixedHeight(MIN_HEIGHT);
    QDateTime start = query.value(2).toDateTime();
    if ( start.isValid() )
    {
      b->setText(query.value(1).toString() + '\n' +
                 start.date().toString() + "    " + start.time().toString("hh:mm"));
      b->setChecked(true);
    }
    else
      b->setText(query.value(1).toString());

    connect(b, SIGNAL(toggled(bool)), &mapper, SLOT(map()));
    mapper.setMapping(b, query.value(0).toInt());
    idButtonMap[query.value(0).toInt()] = b;

    hbox->addWidget(b);
    hbox->setStretchFactor(b, 1);

    // button to change startTime for this employee
    QPushButton *changeStart = new QPushButton(QIcon(":/icons/edit.png"), tr("Start\nändern"));
    changeStart->setFixedHeight(MIN_HEIGHT);
    changeStart->setIconSize(QSize(32, 32));
    hbox->addWidget(changeStart);

    connect(changeStart, SIGNAL(clicked()), &mapper, SLOT(map()));
    mapper.setMapping(changeStart, query.value(0).toInt() + 500);
    if ( !start.isValid() )
      changeStart->setEnabled(false);
    connect(b, SIGNAL(toggled(bool)), changeStart, SLOT(setEnabled(bool)));

    // button to edit whole table of workingTimes for this employee
    b = new QPushButton(QIcon(":/icons/edit.png"), tr("Zeiten"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    hbox->addWidget(b);

    connect(b, SIGNAL(clicked()), &mapper, SLOT(map()));
    mapper.setMapping(b, query.value(0).toInt() + 1000);

    // button to edit whole table of workingTimes for this employee
    b = new QPushButton(QIcon(":/icons/reports.png"), tr("Bericht"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    hbox->addWidget(b);

    connect(b, SIGNAL(clicked()), &mapper, SLOT(map()));
    mapper.setMapping(b, query.value(0).toInt() + 2000);

    ui.employeeLayout->addLayout(hbox);
  }

  connect(&mapper, SIGNAL(mapped(int)), this, SLOT(employeeClicked(int)));
}

//--------------------------------------------------------------------------------

void WorkingTimes::employeeClicked(int id)
{
  QPushButton *b = qobject_cast<QPushButton*>(mapper.mapping(id));
  if ( !b )  // may never happen
    return;

  if ( (id > 500) && (id < 1000) )  // edit startTime button
  {
    id -= 500;

    QSqlQuery query;
    query.exec(QString("SELECT start FROM employees WHERE id=%1").arg(id));
    query.next();
    QDateTime start = query.value(0).toDateTime();
    DateTimeEdit dialog(this, start);
    if ( dialog.exec() == QDialog::Rejected )
      return;
    start = dialog.getDateTime();
    query.exec(QString("UPDATE employees SET start='%1' WHERE id=%2")
                       .arg(start.toString(Qt::ISODate))
                       .arg(id));

    query.exec(QString("SELECT name FROM employees WHERE id=%1").arg(id));
    query.next();
    idButtonMap[id]->setText(query.value(0).toString() + '\n' +
                             start.date().toString() + "    " + start.time().toString("hh:mm"));
    return;
  }

  if ( (id > 1000) && (id < 2000) )  // edit button
  {
    id -= 1000;

    WorkingTimesList dialog(this, id);
    dialog.exec();
    return;
  }

  if ( id > 2000 )  // report
  {
    id -= 2000;

    MonthSelectorDialog dialog(this);
    if ( dialog.exec() == QDialog::Rejected )
      return;
    QDate date = dialog.getDate();
    QDateTime firstDay(QDate(date.year(), date.month(), 1));
    QDateTime lastDay(QDate(date.year(), date.month(), date.daysInMonth()), QTime(23, 59, 59));

    QSqlQuery query;
    query.exec(QString("SELECT name FROM employees WHERE id=%1").arg(id));
    query.next();
    QString employee = query.value(0).toString();

    query.exec(QString("SELECT `start`, `end` FROM `workingTimes`"
                       " WHERE employee=%1 AND (`start` >= '%2') AND (`end` <= '%3') ORDER BY `start`")
                       .arg(id)
                       .arg(firstDay.toString(Qt::ISODate))
                       .arg(lastDay.toString(Qt::ISODate)));

    QString html = "<html><h1>" + employee + " - " + date.toString("MMMM yyyy") + "</h1>";
    html += "<table border='1'>";
    html += QString("<tr> <th>%1</th> <th>%2</th> <th>%3</th> <th>%4</th> </tr>")
                    .arg(tr("Tag"))
                    .arg(tr("Start"))
                    .arg(tr("Ende"))
                    .arg(tr("Stunden"));

    double hours = 0.0;

    while ( query.next() )
    {
      QDateTime start = query.value(0).toDateTime();
      QDateTime end   = query.value(1).toDateTime();
      double span = start.secsTo(end) / 3600.0;
      hours += span;

      html += "<tr>";
      html += "<td style='padding-right: 10px;'><nobr>" + start.toString("ddd d.") + "</nobr></td>";
      html += "<td style='padding-right: 10px; padding-left: 10px;'>" + start.toString("hh:mm") + "</td>";
      html += "<td style='padding-right: 10px; padding-left: 10px;'>" + end.toString("hh:mm") + "</td>";
      html += "<td align='right'>" + QString::number(span, 'f', 2) + "</td>";
      html += "</tr>";
    }

    html += QString("<tr> <td colspan='3'><b>%1</b></td> <td align='right'><b>%2</b></td> </tr>").arg(tr("Summe"))
                    .arg(QString::number(hours, 'f', 2));

    html += "</table>";
    html += "</html>";

    QTextDocument doc;
    doc.setHtml(html);

    ReportView viewer(this, &doc);
    viewer.exec();
    return;
  }

  if ( b->isChecked() )  // starts to work
  {
    QDateTime start = QDateTime::currentDateTime();
    QSqlQuery query(QString("UPDATE employees SET start='%1' WHERE id=%2")
                            .arg(start.toString(Qt::ISODate))
                            .arg(id));

    query.exec(QString("SELECT name FROM employees WHERE id=%1").arg(id));
    query.next();
    b->setText(query.value(0).toString() + '\n' +
               start.date().toString() + "    " + start.time().toString("hh:mm"));
  }
  else  // finishes work -> add new line in times table
  {
    QSqlQuery query(QString("SELECT name, start FROM employees WHERE id=%1").arg(id));
    query.next();
    QDateTime start = query.value(1).toDateTime();
    b->setText(query.value(0).toString());

    query.exec(QString("INSERT INTO workingTimes (employee, start, end) VALUES(%1, '%2', '%3')")
                       .arg(id)
                       .arg(start.toString(Qt::ISODate))
                       .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

    query.exec(QString("UPDATE employees SET start=NULL WHERE id=%1").arg(id));
  }
}

//--------------------------------------------------------------------------------

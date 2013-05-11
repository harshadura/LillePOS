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

#include <PictureButtonSettings.hxx>

#include <QSqlQuery>

//--------------------------------------------------------------------------------

PictureButtonSettings::PictureButtonSettings(QWidget *parent)
  : QDialog(parent), current(0)
{
  ui.setupUi(this);

  connect(ui.listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemClicked(QListWidgetItem*)));

  QSqlQuery query("SELECT value FROM globals WHERE name='numPictureButtons'");
  query.next();
  ui.numButtons->setValue(query.value(0).toInt());

  connect(ui.numButtons, SIGNAL(valueChanged(int)), this, SLOT(setup()));

  setup();
}

//--------------------------------------------------------------------------------

void PictureButtonSettings::setup()
{
  ui.listWidget->clear();
  ui.textEdit->setPlainText("");
  current = 0;

  for (int i = 0; i < ui.numButtons->value(); i++)
  {
    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::UserRole, i);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QSqlQuery query(QString("SELECT strValue FROM globals WHERE name='pictureButton%1'").arg(i));
    if ( query.next() )
    {
      item->setText(query.value(0).toString());
    }
    else
    {
      item->setText(QString::number(i));
      query.exec(QString("INSERT INTO globals (name, strValue) VALUES('pictureButton%1', '%2')").arg(i).arg(QString::number(i)));
      query.exec(QString("INSERT INTO globals (name, strValue) VALUES('pictureText%1', '')").arg(i));
    }

    ui.listWidget->addItem(item);
  }
  QSqlQuery query(QString("UPDATE globals SET value=%1 WHERE name='numPictureButtons'").arg(ui.numButtons->value()));
}

//--------------------------------------------------------------------------------

void PictureButtonSettings::itemClicked(QListWidgetItem *item)
{
  save();
  QSqlQuery query(QString("SELECT strValue FROM globals WHERE name='pictureText%1'")
                          .arg(item->data(Qt::UserRole).toInt()));
  query.next();
  ui.textEdit->setPlainText(query.value(0).toString());
  current = item;
}

//--------------------------------------------------------------------------------

void PictureButtonSettings::save()
{
  if ( !current )
    return;

  QSqlQuery query;

  query.exec(QString("UPDATE globals SET strValue='%2' WHERE name='pictureButton%1'")
                     .arg(current->data(Qt::UserRole).toInt())
                     .arg(current->text()));

  query.exec(QString("UPDATE globals SET strValue='%2' WHERE name='pictureText%1'")
                     .arg(current->data(Qt::UserRole).toInt())
                     .arg(ui.textEdit->toPlainText()));
}

//--------------------------------------------------------------------------------

void PictureButtonSettings::accept()
{
  save();

  QDialog::accept();
}

//--------------------------------------------------------------------------------

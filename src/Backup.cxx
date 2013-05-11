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

#include <Backup.hxx>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>

//--------------------------------------------------------------------------------

Backup::Backup(QWidget *parent, const QString &source, const QUrl &target)
  : QDialog(parent), file(source)
{
  ui.setupUi(this);

  file.open(QIODevice::ReadOnly);

  QNetworkAccessManager *manager = new QNetworkAccessManager(this);

  reply = manager->put(QNetworkRequest(target), &file);

  connect(reply, SIGNAL(uploadProgress(qint64, qint64)),
          this, SLOT(uploadProgress(qint64, qint64)));

  connect(reply, SIGNAL(finished()), this, SLOT(finished()));

  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(error(QNetworkReply::NetworkError)));

  connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  // work around crash
  ui.cancelButton->setEnabled(false);
}

//--------------------------------------------------------------------------------

void Backup::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
  ui.progressBar->setMaximum(bytesTotal);
  ui.progressBar->setValue(bytesSent);
}

//--------------------------------------------------------------------------------

void Backup::error(QNetworkReply::NetworkError)
{
  ui.label->setText(reply->errorString());
  ui.progressBar->hide();
}

//--------------------------------------------------------------------------------

void Backup::finished()
{
  // work around crash
  ui.cancelButton->setEnabled(true);

  if ( ui.progressBar->isHidden() )  // error occured
    return;

  accept();
}

//--------------------------------------------------------------------------------

void Backup::reject()
{
  /* this crashes; see http://bugreports.qt.nokia.com/browse/QTBUG-15859
     We need to wait until the reply emits finished()

    reply->abort();
  */
  QDialog::reject();
}

//--------------------------------------------------------------------------------

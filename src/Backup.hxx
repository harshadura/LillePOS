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

#ifndef _Backup_H_
#define _Backup_H_

#include <QDialog>
#include <QFile>
#include <QNetworkReply>
class QUrl;

#include <ui_Backup.h>

// show a progress bar during the backup file upload to the ftp server

class Backup : public QDialog
{
  Q_OBJECT

  public:
    Backup(QWidget *parent, const QString &source, const QUrl &target);

  public slots:
    virtual void reject();

  private slots:
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();

  private:
    Ui::Backup ui;
    QFile file;
    QNetworkReply *reply;
};

#endif

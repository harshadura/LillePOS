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

#include <PictureView.hxx>

#include <QVBoxLayout>
#include <QLabel>

//--------------------------------------------------------------------------------

PictureView::PictureView(QWidget *parent)
  : QFrame(parent)
{
  setAutoFillBackground(true);
  setBackgroundRole(QPalette::AlternateBase);
  setFrameShape(QFrame::Box);
  QVBoxLayout *vbox = new QVBoxLayout(this);

  titleLabel = new QLabel;
  titleLabel->setAlignment(Qt::AlignHCenter);
  titleLabel->setFont(QFont("Arial", 14));
  titleLabel->setFixedHeight(QFontMetrics(titleLabel->font()).height());

  pixmapLabel = new QLabel;
  pixmapLabel->setScaledContents(true);

  textLabel = new QLabel;

  vbox->addWidget(titleLabel);
  vbox->addWidget(pixmapLabel);
  vbox->addWidget(textLabel);
}

//--------------------------------------------------------------------------------

void PictureView::setPixmap(const QPixmap &pix)
{
  pixmapLabel->setPixmap(pix);
  pixmapLabel->setMaximumWidth(pix.width());
  setMaximumWidth(pix.width());
}

//--------------------------------------------------------------------------------

void PictureView::setText(const QString &txt)
{
  textLabel->setText(txt);
}

//--------------------------------------------------------------------------------

void PictureView::setTitle(const QString &txt)
{
  titleLabel->setText(txt);
}

//--------------------------------------------------------------------------------

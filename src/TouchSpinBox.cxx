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

#include <TouchSpinBox.hxx>

//--------------------------------------------------------------------------------

TouchSpinBox::TouchSpinBox(QWidget *parent)
  : QWidget(parent)
{
  ui.setupUi(this);

  connect(ui.spinBox, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged()));
  connect(ui.plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui.minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
}

//--------------------------------------------------------------------------------

void TouchSpinBox::setEditable(bool editable)
{
  ui.spinBox->setReadOnly(!editable);
}

//--------------------------------------------------------------------------------

void TouchSpinBox::setValue(int v)
{
  ui.spinBox->setValue(v);
}

//--------------------------------------------------------------------------------

void TouchSpinBox::setValue(int v, int min, int max)
{
  ui.spinBox->setMinimum(min);
  ui.spinBox->setMaximum(max);
  ui.spinBox->setValue(v);
}

//--------------------------------------------------------------------------------

int TouchSpinBox::getValue() const
{
  return ui.spinBox->value();
}

//--------------------------------------------------------------------------------

void TouchSpinBox::setMonthMode()
{
  ui.spinBox->setMonthMode();
}

//--------------------------------------------------------------------------------

void TouchSpinBox::plusSlot()
{
  if ( ui.spinBox->value() < ui.spinBox->maximum() )
  {
    ui.spinBox->stepUp();
    emit plusClicked();
  }
}

//--------------------------------------------------------------------------------

void TouchSpinBox::minusSlot()
{
  if ( ui.spinBox->value() > ui.spinBox->minimum() )
  {
    ui.spinBox->stepDown();
    emit minusClicked();
  }
}

//--------------------------------------------------------------------------------

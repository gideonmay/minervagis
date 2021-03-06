
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Tools/ColorButton.h"

#include "QtGui/QPainter"
#include "QtGui/QColorDialog"

using namespace Minerva::QtTools;

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

ColorButton::ColorButton ( QWidget *parent ) :
	BaseClass ( parent ),
	_color ( 255, 255, 255 )
{
  connect ( this, SIGNAL ( clicked() ), this, SLOT ( _buttonClicked() ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

ColorButton::~ColorButton()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the color.
//
///////////////////////////////////////////////////////////////////////////////

void ColorButton::color ( const QColor& c )
{
  if ( c != this->color () && c.isValid() )
  {
    _color = c;
    this->update();
    emit colorChanged();
    emit colorChangedF( c.redF(), c.greenF(), c.blueF(), c.alphaF() );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the color.
//
///////////////////////////////////////////////////////////////////////////////

QColor ColorButton::color () const
{
  return _color;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Paint.
//
///////////////////////////////////////////////////////////////////////////////

void ColorButton::paintEvent( QPaintEvent *e )
{
  // Call the base class first.
  BaseClass::paintEvent ( e );

  if ( this->isEnabled () )
  {
    QPainter painter ( this );
    int margin ( 4 );
    QRect rect ( this->rect().adjusted ( margin, margin, -margin, -margin ) );
    painter.fillRect ( rect, this->color () );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Button has been clicked.
//
///////////////////////////////////////////////////////////////////////////////

void ColorButton::_buttonClicked ()
{
  this->color ( QColorDialog::getColor ( this->color() ) );
}

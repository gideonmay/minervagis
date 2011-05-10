
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "MinervaQt/Plugins/PostGISLayerQt/LineWidget.h"


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

LineWidget::LineWidget ( LineStyle* lineStyle, QWidget* parent ) :
  BaseClass ( parent ),
  _lineStyle ( lineStyle )
{
  this->setupUi ( this );

  if ( _lineStyle.valid () )
    _lineWidth->setValue ( static_cast<int> ( _lineStyle->width() ) );

  connect ( _lineWidth, SIGNAL ( valueChanged ( int ) ), this, SLOT ( _lineWidthChanged ( int ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  The line width has changed.
//
///////////////////////////////////////////////////////////////////////////////

void LineWidget::_lineWidthChanged( int value )
{
  if ( _lineStyle.valid() )
    _lineStyle->width ( value );
}

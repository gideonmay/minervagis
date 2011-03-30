
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "MinervaQt/Plugins/PostGISLayerQt/PolygonWidget.h"

#include "Minerva/Core/Data/LineStyle.h"
#include "Minerva/Core/Data/PolyStyle.h"

#include "Minerva/Qt/Tools/Color.h"

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

PolygonWidget::PolygonWidget ( Style* style, QWidget* parent ) :
  BaseClass ( parent ),
  _style ( style )
{
  this->setupUi ( this );

  if ( _style.valid () )
  {
    Minerva::Core::Data::LineStyle::RefPtr lineStyle ( _style->linestyle() );
    Minerva::Core::Data::PolyStyle::RefPtr polyStyle ( _style->polystyle() );

    if ( polyStyle.valid() )
    {
      _drawBorder->setChecked ( polyStyle->outline() );
      _drawInterior->setChecked ( polyStyle->fill () );
    }

    if ( lineStyle.valid() )
    {
      _borderWidth->setValue ( static_cast<int> ( lineStyle->width () ) );
      _borderColor->color ( Minerva::QtTools::Color< Usul::Math::Vec4f >::convert ( lineStyle->color() ) );
    }
  }

  connect ( _drawBorder, SIGNAL ( stateChanged ( int ) ), this, SLOT ( _drawBorderChanged ( int ) ) );
  connect ( _drawInterior, SIGNAL ( stateChanged ( int ) ), this, SLOT ( _drawInteriorChanged ( int ) ) );
  connect ( _borderWidth, SIGNAL ( valueChanged ( int ) ), this, SLOT ( _borderWidthChanged ( int ) ) );
  connect ( _borderColor, SIGNAL ( colorChanged () ), this, SLOT ( _borderColorChanged ( ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Draw border has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PolygonWidget::_drawBorderChanged ( int state )
{
  if ( _style.valid() )
  {
    Minerva::Core::Data::PolyStyle::RefPtr polyStyle ( _style->polystyle() );
    if ( polyStyle.valid() )
    {
      polyStyle->outline ( state > 0 );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Draw interior has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PolygonWidget::_drawInteriorChanged ( int state )
{
  if ( _style.valid() )
  {
    Minerva::Core::Data::PolyStyle::RefPtr polyStyle ( _style->polystyle() );
    if ( polyStyle.valid() )
    {
      polyStyle->fill ( state > 0 );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Border width has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PolygonWidget::_borderWidthChanged ( int value )
{
  if ( _style.valid() )
  {
    Minerva::Core::Data::LineStyle::RefPtr lineStyle ( _style->linestyle() );
    if ( lineStyle.valid() )
    {
      lineStyle->width ( value );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Border color has changed.
//
///////////////////////////////////////////////////////////////////////////////

void PolygonWidget::_borderColorChanged ()
{
  if ( _style.valid() )
  {
    Minerva::Core::Data::LineStyle::RefPtr lineStyle ( _style->linestyle() );
    if ( lineStyle.valid() )
    {
      lineStyle->color ( Minerva::QtTools::Color< Usul::Math::Vec4f >::convert ( _borderColor->color() ) );
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Should the polygon data be rasterized?
//
///////////////////////////////////////////////////////////////////////////////

bool PolygonWidget::rasterize() const
{
  return Qt::Checked == _rasterize->checkState();
}

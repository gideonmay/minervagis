
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __POSTGIS_POLYGON_WIDGET_H__
#define __POSTGIS_POLYGON_WIDGET_H__

#include "ui_PolygonWidget.h"
#include "Minerva/Core/Data/Style.h"

#include "QtGui/QWidget"

class PolygonWidget : public QWidget,
  private Ui::PolygonWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef Minerva::Core::Data::Style Style;

  PolygonWidget ( Style* style, QWidget* parent = 0x0 );

  bool rasterize() const;

protected slots:
  void _drawBorderChanged ( int state );
  void _drawInteriorChanged ( int state );
  void _borderWidthChanged ( int value );
  void _borderColorChanged ();

private:
  Minerva::Core::Data::Style::RefPtr _style;
};


#endif // __POSTGIS_POLYGON_WIDGET_H__


///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __POSTGIS_LINE_WIDGET_H__
#define __POSTGIS_LINE_WIDGET_H__

#include "ui_LineWidget.h"
#include "Minerva/Core/Data/LineStyle.h"

#include "QtGui/QWidget"

class LineWidget : public QWidget,
                   private Ui::LineWidget
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef Minerva::Core::Data::LineStyle LineStyle;

  LineWidget ( LineStyle* layer, QWidget* parent = 0x0 );

protected slots:
  void _lineWidthChanged ( int value );

private:
  LineStyle::RefPtr _lineStyle;
};

#endif // __POSTGIS_LINE_WIDGET_H__

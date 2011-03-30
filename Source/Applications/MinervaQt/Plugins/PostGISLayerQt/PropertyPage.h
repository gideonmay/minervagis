
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __POSTGIS_COLOR_PAGE_H__
#define __POSTGIS_COLOR_PAGE_H__

#include "ui_LayerProperties.h"

#include "Minerva/Plugins/GDAL/ConnectionInfo.h"
#include "Minerva/Plugins/GDAL/PostGISLayer.h"

#include "Minerva/Qt/Tools/ColorButton.h"

#include "QtGui/QWidget"

#include <vector>
#include <string>

class AddPostGISLayerWidget;

class PropertyPage : public QWidget,
                     private Ui::LayerProperties
{
  Q_OBJECT;
public:
  typedef QWidget BaseClass;
  typedef Minerva::Layers::GDAL::ConnectionInfo Connection;
  typedef Minerva::Layers::GDAL::PostGISLayer  Layer;
  typedef std::vector < std::string > Strings;

  PropertyPage ( Layer *layer, QWidget *parent = 0x0 );
  virtual ~PropertyPage ();

  bool rasterize() const;
protected:
  void _initDrawingTab();
  void _initGeneralTab();

  void _initLabelProperties();

protected slots:

  /// The name for the layer has changed.
  void _nameChanged();

  /// Slots for label.
  void _labelShownChanged( int state );
  void _labelColorChanged();
  void _labelColumnChanged( int index );
  void _labelSizeChanged( int value );
  void _labelZOffsetChanged ( int value );

private:
  Layer::RefPtr _layer;
  QWidget *_colorWidget;
  QWidget *_primitiveWidget;
};


#endif // __POSTGIS_COLOR_PAGE_H__

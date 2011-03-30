
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author(s): Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "MinervaQt/Plugins/PostGISLayerQt/AddPostGISLayerWidget.h"
#include "MinervaQt/Plugins/PostGISLayerQt/DatabasePage.h"
#include "MinervaQt/Plugins/PostGISLayerQt/PropertyPage.h"

#include "Minerva/Core/Data/Container.h"
#include "Minerva/Plugins/GDAL/RasterPolygonLayer.h"

#include <iostream>

#include "QtGui/QVBoxLayout"
#include "QtGui/QHBoxLayout"
#include "QtGui/QPushButton"

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

AddPostGISLayerWidget::AddPostGISLayerWidget ( QWidget *parent ) : 
  BaseClass ( parent ),
  _databasePage ( 0x0 ),
  _propertyPage ( 0x0 ),
  _editButton ( new QPushButton ( "Edit Properties" ) ),
  _stackedWidget ( 0x0 )
{
  QVBoxLayout *topLayout ( new QVBoxLayout );
  this->setLayout ( topLayout );

  _stackedWidget = new QStackedWidget ( this );

  QHBoxLayout *layout ( new QHBoxLayout );
  layout->addStretch ();
  layout->addWidget ( _editButton );

  topLayout->addWidget ( _stackedWidget );
  topLayout->addLayout ( layout );

  _databasePage = new DatabasePage ( this );

  _stackedWidget->addWidget ( _databasePage );
  _stackedWidget->setCurrentWidget ( _databasePage );

  _editButton->setEnabled ( false );

  connect ( _databasePage, SIGNAL ( layerChanged ( bool ) ), _editButton, SLOT ( setEnabled ( bool ) ) );
  connect ( _editButton, SIGNAL ( clicked () ), this, SLOT ( _editLayerProperties () ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

AddPostGISLayerWidget::~AddPostGISLayerWidget()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add images to caller.
//
///////////////////////////////////////////////////////////////////////////////

void AddPostGISLayerWidget::apply ( Minerva::Core::Data::Feature* parent )
{
  Minerva::Core::Data::Container::RefPtr container ( 0x0 != parent ? parent->asContainer() : 0x0 );
  if ( !container )
    return;

  bool rasterize ( false );
  
  if ( 0x0 != _propertyPage )
  {
    rasterize = _propertyPage->rasterize();
  }
  
  // Return now if we don't have a database page.
  if ( 0x0 == _databasePage )
    return;
  
  Minerva::Core::Data::Feature::RefPtr layer ( _databasePage->layer() );
  
  if ( rasterize )
  {
    layer = new Minerva::Layers::GDAL::RasterPolygonLayer ( _databasePage->layer() );
  }

  container->add ( layer );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Edit layer properties.
//
///////////////////////////////////////////////////////////////////////////////

void AddPostGISLayerWidget::_editLayerProperties ()
{
  if ( 0x0 != _propertyPage )
  {
    _stackedWidget->removeWidget ( _propertyPage );
    delete _propertyPage;
  }

  Minerva::Layers::GDAL::PostGISLayer::RefPtr layer ( 0x0 != _databasePage ? _databasePage->layer() : 0x0 );
  if ( layer.valid() )
  {
    _propertyPage = new PropertyPage ( layer.get(), this );
    _stackedWidget->addWidget ( _propertyPage );
    _stackedWidget->setCurrentWidget ( _propertyPage );
  }

  _editButton->setEnabled ( false );
}

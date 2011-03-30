
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Created by: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////

#include "MinervaQt/Plugins/PostGISLayerQt/PostGISLayerQtComponent.h"
#include "MinervaQt/Plugins/PostGISLayerQt/PropertyPage.h"
#include "Minerva/Plugins/GDAL/PostGISLayer.h"

#include "Usul/Components/Factory.h"

#include "QtGui/QDialog"
#include "QtGui/QVBoxLayout"
#include "QtGui/QHBoxLayout"
#include "QtGui/QPushButton"

USUL_DECLARE_COMPONENT_FACTORY ( PostGISLayerQtComponent );
USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( PostGISLayerQtComponent, PostGISLayerQtComponent::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

PostGISLayerQtComponent::PostGISLayerQtComponent() : BaseClass(),
  _widget ( 0x0 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

PostGISLayerQtComponent::~PostGISLayerQtComponent() 
{
//  delete _widget;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for the interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *PostGISLayerQtComponent::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Usul::Interfaces::IPlugin::IID:
    return static_cast < Usul::Interfaces::IPlugin* > ( this );
  case Minerva::Interfaces::ILayerAddGUIQt::IID:
    return static_cast < Minerva::Interfaces::ILayerAddGUIQt * > ( this );
  case Minerva::Interfaces::ILayerModifyGUIQt::IID:
    return static_cast < Minerva::Interfaces::ILayerModifyGUIQt * > ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Usul::Interfaces::IPlugin implementation
//
///////////////////////////////////////////////////////////////////////////////

std::string PostGISLayerQtComponent::getPluginName() const 
{
  return "PostGIS Layer Qt GUI";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the widget that adds a layer.
//
///////////////////////////////////////////////////////////////////////////////

QWidget* PostGISLayerQtComponent::layerAddGUI()
{
  _widget = new AddPostGISLayerWidget;
  return _widget;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the name.
//
///////////////////////////////////////////////////////////////////////////////

std::string PostGISLayerQtComponent::name() const 
{
  return "PostGIS";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Apply the caller to the widget.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayerQtComponent::apply ( Minerva::Core::Data::Feature* parent, DataLoadedCallback callback )
{
  if ( 0x0 != _widget )
    _widget->apply ( parent );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Do we handle this type of layer?
//
///////////////////////////////////////////////////////////////////////////////

bool PostGISLayerQtComponent::handle ( Minerva::Core::Data::Feature* feature ) const
{
  return 0x0 != dynamic_cast < Minerva::Layers::GDAL::PostGISLayer * > ( feature );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show the property gui.
//
///////////////////////////////////////////////////////////////////////////////

void PostGISLayerQtComponent::showModifyGUI ( Minerva::Core::Data::Feature* feature, Minerva::Core::Data::Feature* parent, Usul::Interfaces::IUnknown* caller ) 
{
  Minerva::Layers::GDAL::PostGISLayer::RefPtr baseLayer ( dynamic_cast<Minerva::Layers::GDAL::PostGISLayer*> ( feature ) );
  Minerva::Core::Data::Container::RefPtr container ( 0x0 != parent ? parent->asContainer() : 0x0 );

  // Return now if no layer.
  if ( 0x0 == baseLayer || 0x0 == container )
    return;

  // Make a copy.
  Minerva::Core::Data::Feature::RefPtr clone ( baseLayer->clone() );

  Minerva::Layers::GDAL::PostGISLayer::RefPtr clonedLayer ( dynamic_cast<Minerva::Layers::GDAL::PostGISLayer*> ( clone.get() ) );

  PropertyPage *page ( new PropertyPage ( clonedLayer ) );

  QDialog dialog ( 0x0 );
  QPushButton *ok ( new QPushButton ( "Ok" ) );
  QPushButton *cancel ( new QPushButton ( "Cancel" ) );

  QObject::connect ( ok,     SIGNAL ( clicked() ), &dialog, SLOT ( accept() ) );
  QObject::connect ( cancel, SIGNAL ( clicked() ), &dialog, SLOT ( reject() ) );

  QVBoxLayout *topLayout ( new QVBoxLayout );
  dialog.setLayout ( topLayout );

  QHBoxLayout *hLayout ( new QHBoxLayout );

  topLayout->addWidget ( page );
  topLayout->addLayout ( hLayout );
  
  hLayout->addStretch();
  hLayout->addWidget ( ok );
  hLayout->addWidget ( cancel );

  if ( QDialog::Accepted == dialog.exec() )
  {
    // Remove the old one.
    container->remove ( baseLayer );

    // Add the new one.
    container->add ( clonedLayer );
  }
}

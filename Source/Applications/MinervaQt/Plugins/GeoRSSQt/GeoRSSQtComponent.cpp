
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

#include "MinervaQt/Plugins/GeoRSSQt/GeoRSSQtComponent.h"
#include "MinervaQt/Plugins/GeoRSSQt/GeoRSSItemWidget.h"
#include "Minerva/Plugins/GeoRSS/GeoRSSLayer.h"
#include "Minerva/Plugins/GeoRSS/Item.h"

#include "Usul/Components/Factory.h"

#include "QtGui/QDialog"
#include "QtGui/QDialogButtonBox"
#include "QtGui/QVBoxLayout"

USUL_DECLARE_COMPONENT_FACTORY ( GeoRSSQtComponent );
USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( GeoRSSQtComponent, GeoRSSQtComponent::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////////////////////////

GeoRSSQtComponent::GeoRSSQtComponent() : BaseClass(),
  _widget ( 0x0 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////////////////////////

GeoRSSQtComponent::~GeoRSSQtComponent() 
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for the interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *GeoRSSQtComponent::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
  case Usul::Interfaces::IUnknown::IID:
  case Usul::Interfaces::IPlugin::IID:
    return static_cast < Usul::Interfaces::IPlugin* > ( this );
  case Minerva::Common::ILayerAddGUIQt::IID:
    return static_cast < Minerva::Common::ILayerAddGUIQt * > ( this );
  case Minerva::Common::ILayerModifyGUIQt::IID:
    return static_cast < Minerva::Common::ILayerModifyGUIQt * > ( this );
  default:
    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Usul::Interfaces::IPlugin implementation
//
///////////////////////////////////////////////////////////////////////////////

std::string GeoRSSQtComponent::getPluginName() const 
{
  return "GeoRSS Qt GUI";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the widget to add the gui.
//
///////////////////////////////////////////////////////////////////////////////

QWidget* GeoRSSQtComponent::layerAddGUI()
{
  _widget = new AddGeoRSSLayerWidget;
  return _widget;
}


///////////////////////////////////////////////////////////////////////////////
//
//  The name of the layer type.
//
///////////////////////////////////////////////////////////////////////////////

std::string GeoRSSQtComponent::name() const 
{
  return "GeoRSS";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add Geo RSS feed to caller.
//
///////////////////////////////////////////////////////////////////////////////

void GeoRSSQtComponent::apply ( Minerva::Core::Data::Feature *feature, DataLoadedCallback dataLoadedCallback )
{
  Minerva::Core::Data::Container::RefPtr parent ( 0x0 != feature ? feature->asContainer() : 0x0 );
  if ( 0x0 != _widget && true == parent.valid() )
  {
    const std::string url ( _widget->url() );
    
    // Make sure we have a url.
    if ( false == url.empty() )
    {
      Minerva::Layers::GeoRSS::GeoRSSLayer::RefPtr layer ( new Minerva::Layers::GeoRSS::GeoRSSLayer );
      
      // Set the new members.
      GeoRSSQtComponent::_setLayerMembers ( *_widget, *layer );
      
      // Start the download.
      layer->downloadFeed();
      
      // Add the layer.
      parent->add ( layer );
    }
  }

  _widget = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Do we handle this layer.
//
///////////////////////////////////////////////////////////////////////////////

bool GeoRSSQtComponent::handle ( Minerva::Core::Data::Feature *feature ) const
{
  const bool isGeoRssLayer ( 0x0 != dynamic_cast<Minerva::Layers::GeoRSS::GeoRSSLayer*> ( feature ) );
  
#ifdef _DEBUG
  const bool isGeoRssItem  ( 0x0 != dynamic_cast<Minerva::Layers::GeoRSS::Item*> ( feature ) );
  return isGeoRssLayer || isGeoRssItem;
#else
  return isGeoRssLayer;
#endif
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show a dialog to modify properties.
//
///////////////////////////////////////////////////////////////////////////////

void GeoRSSQtComponent::showModifyGUI ( Minerva::Core::Data::Feature *feature, Minerva::Core::Data::Feature* parent, Usul::Interfaces::IUnknown* caller )
{
  typedef Minerva::Layers::GeoRSS::GeoRSSLayer GeoRSSLayer;
  
  GeoRSSLayer *geoRss ( dynamic_cast<GeoRSSLayer*> ( feature ) );
  
  if ( 0x0 != geoRss )
  {
    this->_displayGeoRSSLayerGUI ( *geoRss, caller );
  }
  
  else if ( GeoRSSItem *item = dynamic_cast<GeoRSSItem*> ( feature ) )
  {
    this->_displayGeoRSSItemGUI ( *item, caller );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the layer members.
//
///////////////////////////////////////////////////////////////////////////////

void GeoRSSQtComponent::_setLayerMembers ( AddGeoRSSLayerWidget& widget, GeoRSSLayer& layer )
{
  // Set the new properties.
  layer.url ( widget.url() );
  layer.refreshRate ( widget.refreshRate() );
  layer.filteringEnabled ( widget.enableFiltering() );
  layer.filter ( GeoRSSLayer::Filter ( widget.element(), widget.value() ) );
  layer.color ( widget.color() );
  layer.maximumItems ( widget.maximumItems() );
  layer.maximumAge ( widget.maximumAge() );
  layer.useRegEx ( widget.useRegEx() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show the GUI to edit the GeoRSSLayer.
//
///////////////////////////////////////////////////////////////////////////////

void GeoRSSQtComponent::_displayGeoRSSLayerGUI ( GeoRSSLayer& geoRss, Usul::Interfaces::IUnknown* caller )
{
  QDialog dialog ( 0x0 );
  
  // Make page to edit geoRSS values.
  AddGeoRSSLayerWidget *page ( new AddGeoRSSLayerWidget ( &dialog ) );
  page->url ( geoRss.url() );
  page->refreshRate ( geoRss.refreshRate() );
  page->enableFiltering ( geoRss.filteringEnabled() );
  page->element ( geoRss.filter().first );
  page->value ( geoRss.filter().second );
  page->color ( geoRss.color() );
  page->maximumItems ( geoRss.maximumItems() );
  page->maximumAge ( geoRss.maximumAge() );
  page->useRegEx ( geoRss.useRegEx() );
  
  const QDialogButtonBox::StandardButtons buttons ( QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok );
  QDialogButtonBox *buttonBox ( new QDialogButtonBox ( buttons, Qt::Horizontal, &dialog ) );
  
  QObject::connect ( buttonBox, SIGNAL ( accepted() ), &dialog, SLOT ( accept() ) );
  QObject::connect ( buttonBox, SIGNAL ( rejected() ), &dialog, SLOT ( reject() ) );
  
  QVBoxLayout *topLayout ( new QVBoxLayout );
  dialog.setLayout ( topLayout );
  
  topLayout->addWidget ( page );
  topLayout->addWidget ( buttonBox );
  
  // Set the title.
  dialog.setWindowTitle ( QString ( "Edit " ) + QString ( geoRss.name().c_str() ) );
  
	const int result ( dialog.exec() );
  
	if ( QDialog::Accepted == result )
	{
    // Set the new members.
    GeoRSSQtComponent::_setLayerMembers ( *page, geoRss );
    
    // Start the download.
    geoRss.downloadFeed();
	}
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show the GUI to edit the GeoRSSItem.
//
///////////////////////////////////////////////////////////////////////////////

void GeoRSSQtComponent::_displayGeoRSSItemGUI ( GeoRSSItem& item, Usul::Interfaces::IUnknown* caller )
{
  QDialog dialog ( 0x0 );
  
  // Make page to edit geoRSS values.
  GeoRSSItemWidget *page ( new GeoRSSItemWidget ( &dialog ) );
  page->title ( item.name() );
  page->description ( item.description() );
  
  const QDialogButtonBox::StandardButtons buttons ( QDialogButtonBox::NoButton|QDialogButtonBox::Ok );
  QDialogButtonBox *buttonBox ( new QDialogButtonBox ( buttons, Qt::Horizontal, &dialog ) );
  
  QObject::connect ( buttonBox, SIGNAL ( accepted() ), &dialog, SLOT ( accept() ) );
  
  QVBoxLayout *topLayout ( new QVBoxLayout );
  dialog.setLayout ( topLayout );
  
  topLayout->addWidget ( page );
  topLayout->addWidget ( buttonBox );
  
  // Set the title.
  dialog.setWindowTitle ( QString ( item.name().c_str() ) );
  
	dialog.exec();
}

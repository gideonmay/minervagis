
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

#include "MinervaQt/Plugins/WmsLayerQt/WmsLayerQtComponent.h"

#include "Minerva/Qt/Widgets/EditWmsLayerWidget.h"
#include "Minerva/Qt/Widgets/AddNetworkLayerWidget.h"

#include "Usul/Components/Factory.h"
#include "Usul/Registry/Database.h"
#include "Usul/Registry/Qt.h"

#include "QtGui/QDialog"
#include "QtGui/QDialogButtonBox"
#include "QtGui/QVBoxLayout"

USUL_DECLARE_COMPONENT_FACTORY ( WmsLayerQtComponent );
USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( WmsLayerQtComponent, WmsLayerQtComponent::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
//  Registry constants.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail
{
  const std::string SECTION  ( "wms_qt_gui" );
  const std::string GEOMETRY ( "geometry" );
}


///////////////////////////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////////////////////////

WmsLayerQtComponent::WmsLayerQtComponent() : BaseClass(),
  _widget ( 0x0 )
{
}


///////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////////////////////////

WmsLayerQtComponent::~WmsLayerQtComponent() 
{
//  delete _widget;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for the interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown *WmsLayerQtComponent::queryInterface ( unsigned long iid )
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

std::string WmsLayerQtComponent::getPluginName() const 
{
  return "WMS Layer Qt GUI";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Widget to add a gui for wms servers.
//
///////////////////////////////////////////////////////////////////////////////

QWidget* WmsLayerQtComponent::layerAddGUI()
{
  // TODO: Need to clean up if cancelled.
  _widget = new AddWmsLayerWidget;
  return _widget;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the name of widget.  This will go on a tab.
//
///////////////////////////////////////////////////////////////////////////////

std::string WmsLayerQtComponent::name() const 
{
  return "WMS";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Apply any selections the user made.
//
///////////////////////////////////////////////////////////////////////////////

void WmsLayerQtComponent::apply ( Minerva::Core::Data::Feature* parent, DataLoadedCallback dataLoadedCallback )
{
  if ( 0x0 != _widget )
    _widget->apply ( parent );

  // TODO: What about cancelled dialog?
  _widget = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Do we handle this layer.
//
///////////////////////////////////////////////////////////////////////////////

bool WmsLayerQtComponent::handle ( Minerva::Core::Data::Feature* feature ) const
{
  return 0x0 != dynamic_cast<Minerva::Core::Layers::RasterLayerWms*> ( feature );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Show a dialog to modify properties.
//
///////////////////////////////////////////////////////////////////////////////

void WmsLayerQtComponent::showModifyGUI ( Minerva::Core::Data::Feature* feature, Minerva::Core::Data::Feature* parent, Usul::Interfaces::IUnknown* caller )
{
	typedef Minerva::Core::Layers::RasterLayerWms RasterLayerWms;
	typedef RasterLayerWms::Options Options;
	typedef RasterLayerWms::Alphas Alphas;

  RasterLayerWms::RefPtr wms ( dynamic_cast <RasterLayerWms*> ( feature ) );
  
  // Return now if no layer.
  if ( 0x0 == wms )
    return;
  
  // Save current state.
	Alphas alphas ( wms->alphas() );
	Options options ( wms->options() );
  std::string name ( wms->name() );
  std::string url ( wms->urlBase() );
  
  typedef Minerva::QtWidgets::EditWmsLayerWidget EditWmsLayerWidget;
  EditWmsLayerWidget *page ( new EditWmsLayerWidget ( wms ) );
  
  QDialog dialog ( 0x0 );
  
  const QDialogButtonBox::StandardButtons buttons ( QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok );
  QDialogButtonBox *buttonBox ( new QDialogButtonBox ( buttons, Qt::Horizontal, &dialog ) );
  
  QObject::connect ( buttonBox, SIGNAL ( accepted() ), &dialog, SLOT ( accept() ) );
  QObject::connect ( buttonBox, SIGNAL ( rejected() ), &dialog, SLOT ( reject() ) );
  
  QVBoxLayout *topLayout ( new QVBoxLayout );
  dialog.setLayout ( topLayout );
  
  topLayout->addWidget ( page );
  topLayout->addWidget ( buttonBox );
  
  // Set the title.
  dialog.setWindowTitle( QString ( "Edit " ) + QString ( name.c_str() ) );
  
  // Set the window's properties.
  dialog.restoreGeometry ( Usul::Registry::Database::instance()[Detail::SECTION][Detail::GEOMETRY].get<QByteArray> ( QByteArray() ) );
  
	const int result ( dialog.exec() );

	if ( QDialog::Rejected == result )
  {
    // Restore the state.
		wms->alphas ( alphas );
		wms->options ( options );
    wms->name ( name );
    wms->urlBase ( url );
  }  
  
  // Save the window's properties.
  Usul::Registry::Database::instance()[Detail::SECTION][Detail::GEOMETRY] = dialog.saveGeometry();
}

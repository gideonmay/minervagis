
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Qt/Widgets/FlyTo.h"

#include "Minerva/Network/GeoCode.h"

#include "Usul/Strings/Format.h"

#include "QtGui/QLineEdit"
#include "QtGui/QPushButton"
#include "QtGui/QVBoxLayout"

#include <iostream>
#include <sstream>

using namespace Minerva::QtWidgets;

///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

FlyTo::FlyTo ( QWidget* parent ) : BaseClass ( parent ),
  _lineEdit ( new QLineEdit )
{
  QPushButton *go ( new QPushButton ( "Go" ) );
  
  connect ( go,        SIGNAL ( clicked() ),       this, SLOT ( _onFlyToClicked() ) );
  connect ( _lineEdit, SIGNAL ( returnPressed() ), this, SLOT ( _onFlyToClicked() ) );
  
  QHBoxLayout *topLayout ( new QHBoxLayout );
  this->setLayout ( topLayout );
  
  topLayout->addWidget ( _lineEdit );
  topLayout->addWidget ( go );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

FlyTo::~FlyTo()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Fly to location.
//
///////////////////////////////////////////////////////////////////////////////

void FlyTo::_onFlyToClicked()
{
  const std::string location ( _lineEdit->text().toStdString() );
  
  if ( true == location.empty() )
    return;

  typedef Minerva::Network::GeoCode::Result GeoCodeResult;
  Minerva::Network::GeoCode geoCode;
  GeoCodeResult result ( geoCode ( location ) );
  if ( false == result.success )
  {
    std::cout << "Could not geocode " << location << std::endl;
    return;
  }

  std::ostringstream out;
  out << ( ( result.address.empty() ) ? std::string() : result.address );
  out << ( ( result.city.empty()    ) ? std::string() : ( ( out.str().empty() ) ? result.city    : Usul::Strings::format ( ", ", result.city    ) ) );
  out << ( ( result.state.empty()   ) ? std::string() : ( ( out.str().empty() ) ? result.state   : Usul::Strings::format ( ", ", result.state   ) ) );
  out << ( ( result.zip.empty()     ) ? std::string() : ( ( out.str().empty() ) ? result.zip     : Usul::Strings::format ( ", ", result.zip     ) ) );
  out << ( ( result.country.empty() ) ? std::string() : ( ( out.str().empty() ) ? result.country : Usul::Strings::format ( ", ", result.country ) ) );

  const std::string address ( ( out.str().empty() ) ? location : out.str() );
  std::cout << Usul::Strings::format ( "Geocoded '", address, "' to (", result.location[1], ',', result.location[0], ")\n" ) << std::flush;
  
  emit flyToLocation ( result.location[0], result.location[1] );
}

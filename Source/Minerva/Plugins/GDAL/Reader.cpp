
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Adam Kubach
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/Reader.h"
#include "Minerva/Plugins/GDAL/RasterLayerGDAL.h"

#include "Minerva/Core/Factory/Readers.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/filesystem.hpp"

using namespace Minerva::Layers::GDAL;

///////////////////////////////////////////////////////////////////////////////
//
//  Register readers with the factory.
//
///////////////////////////////////////////////////////////////////////////////

namespace
{
  namespace MF = Minerva::Core::Factory;
  MF::RegisterReader _readerForGDAL ( new Reader );
}

USUL_IMPLEMENT_IUNKNOWN_MEMBERS ( Reader, Reader::BaseClass );


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

Reader::Reader() : BaseClass(),
  _filters()
{
  _filters.push_back ( Filter ( "Arc Ascii (*.asc)", "*.asc" ) );
  _filters.push_back ( Filter ( "Arc Binary (*.adf)", "*.adf" ) );
  _filters.push_back ( Filter ( "Digital Elevation Model (*.dem)", "*.dem" ) );
  _filters.push_back ( Filter ( "NASA SRTM (*.hgt)", "*.hgt" ) );
  _filters.push_back ( Filter ( "TIFF (*.tiff *.tif)", "*.tiff,*.tif" ) );
  _filters.push_back ( Filter ( "JEPG (*.jpg)", "*.jpg" ) );
  _filters.push_back ( Filter ( "PNG (*.png)", "*.png" ) );
  _filters.push_back ( Filter ( "GDAL Virtual Format (*.vrt)", ".vrt" ) );
  _filters.push_back ( Filter ( "National Transimission Format (*.ntf)", ".ntf" ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

Reader::~Reader()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Query for interface.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Interfaces::IUnknown* Reader::queryInterface ( unsigned long iid )
{
  switch ( iid )
  {
    case Usul::Interfaces::IUnknown::IID:
    case Minerva::Common::IReadFeature::IID:
      return static_cast<Minerva::Common::IReadFeature*> ( this );
    default:
      return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the filters for this reader.
//
///////////////////////////////////////////////////////////////////////////////

Reader::Filters Reader::filters() const
{
  return _filters;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Can this reader handle the extension?
//
///////////////////////////////////////////////////////////////////////////////

bool Reader::canHandle ( const std::string& extension ) const
{
  for ( Filters::const_iterator iter = _filters.begin(); iter != _filters.end(); ++iter )
  {
    const std::string ext ( boost::filesystem::extension ( boost::algorithm::to_lower_copy ( iter->second ) ) );
    if ( ext == extension )
    {
      return true;
    }
  }
  
  return false;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature* Reader::readFeature ( const std::string& filename )
{
  RasterLayerGDAL::RefPtr feature ( new RasterLayerGDAL );
  feature->read ( filename );
  return feature.release();
}

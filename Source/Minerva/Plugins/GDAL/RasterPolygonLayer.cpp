
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/RasterPolygonLayer.h"
#include "Minerva/Plugins/GDAL/Common.h"
#include "Minerva/Plugins/GDAL/Convert.h"
#include "Minerva/Plugins/GDAL/ConnectionInfo.h"

#include "Minerva/Core/Data/LineStyle.h"

#include "Usul/App/Application.h"
#include "Usul/Factory/RegisterCreator.h"
#include "Usul/File/Path.h"
#include "Usul/Math/Absolute.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Scope/RemoveFile.h"
#include "Usul/Strings/Format.h"
#include "Usul/Threads/Safe.h"

#include "boost/bind.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/filesystem.hpp"

#include "osgDB/ReadFile"
#include "osgDB/WriteFile"

#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "ogr_api.h"
#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "cpl_error.h"

using namespace Minerva::Layers::GDAL;

USUL_FACTORY_REGISTER_CREATOR ( RasterPolygonLayer );


///////////////////////////////////////////////////////////////////////////////
//
//  Default Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterPolygonLayer::RasterPolygonLayer () : 
  BaseClass(),
  _layer ( 0x0 ),
  _dir ( Usul::File::Temp::directory() ),
  _projectionText(),
  _initialized ( false ),
  _geometries(),
  _burnValues()
{
  this->_addMember ( "Layer", _layer );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterPolygonLayer::RasterPolygonLayer ( Layer* layer ) : 
  BaseClass(),
  _layer ( layer ),
  _dir ( Usul::File::Temp::directory() ),
  _projectionText(),
  _initialized ( false ),
  _geometries(),
  _burnValues()
{
  this->_addMember ( "Layer", _layer );
  
  if ( _layer.valid() )
    this->name ( _layer->name() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterPolygonLayer::RasterPolygonLayer ( const RasterPolygonLayer& rhs ) :
  BaseClass ( rhs ),
  _layer ( rhs._layer ),
  _dir ( rhs._dir ),
  _projectionText( rhs._projectionText ),
  _initialized ( false ),
  _geometries(),
  _burnValues()
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterPolygonLayer::~RasterPolygonLayer()
{
  // Clean up geometries.
  for ( Geometries::iterator iter = _geometries.begin(); iter != _geometries.end(); ++iter )
    delete *iter;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Initialize.
//
///////////////////////////////////////////////////////////////////////////////

void RasterPolygonLayer::_init()
{
  bool initialized ( Usul::Threads::Safe::get( this->mutex(), _initialized ) );

  // Return if we are already initialized.
  if ( true == initialized )
    return;

  // Set an error handler.
  Detail::PushPopErrorHandler handler;
  
  Guard guard ( this );

  // Return now if we don't have a valid layer to work with.
  if ( false == _layer.valid() )
    return;
  
  // Set the name.
  this->name ( _layer->name() );
  
  // Get the projection text.
  _projectionText = _layer->projectionWKT();
  
  // Get the extents of the layer.
  Extents extents ( _layer->calculateExtents() );
  
  // Make the transform.
  OGRSpatialReference src ( _projectionText.c_str() ), dst;
  dst.SetWellKnownGeogCS ( "WGS84" );
  OGRCoordinateTransformation *transform ( OGRCreateCoordinateTransformation( &src, &dst ) );
  
  // Make sure the transformation is destroyed.
  Usul::Scope::Caller::RefPtr destroyTransform ( Usul::Scope::makeCaller ( boost::bind<void> ( ::OCTDestroyCoordinateTransformation, transform ) ) );

  // Transform the extents.
  if ( 0x0 != transform )
  {
    Extents::Vertex ll ( extents.minimum() ), ur ( extents.maximum() );
    transform->Transform ( 1, &ll[0], &ll[1] );
    transform->Transform ( 1, &ur[0], &ur[1] );
    extents = Extents ( ll, ur );
  }
  
  // Set the extents.
  this->extents ( extents );

  this->_initGeometries();

  // We are initialized.
  Usul::Threads::Safe::set( this->mutex(), true, _initialized );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the geometries from the server..
//
///////////////////////////////////////////////////////////////////////////////

void RasterPolygonLayer::_initGeometries()
{
  // Set an error handler.
  Detail::PushPopErrorHandler handler;

  Guard guard ( this );

  // Return if no layer.
  if ( false == _layer.valid() )
    return;

  // Get the connection.
  ConnectionInfo::RefPtr connection ( _layer->connection() );
    
  // Return if no connection.
  if ( false == connection.valid() )
    return;
    
  const std::string driverName ( "PostgreSQL" );
  OGRSFDriver *driver ( OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName( driverName.c_str() ) );
    
  // Return if we didn't find a driver.
  if ( 0x0 == driver )
    return;

  std::string name ( Usul::Strings::format ( "PG:", connection->connectionString() ) );
    
  // Get the data source.
  OGRDataSource *vectorData ( driver->CreateDataSource( name.c_str(), 0x0 ) );
    
  // Return if no data.
  if ( 0x0 == vectorData )
    return;

  // Make the query.
  std::string query ( Usul::Strings::format ( "SELECT * ", " FROM ", _layer->tablename() ) );
  
  // Make a layer.
  OGRLayer *layer ( vectorData->ExecuteSQL( query.c_str(), 0x0, 0x0 ) );
  
  if ( 0x0 == layer )
    return;
  
  OGRFeature* feature ( 0x0 );
  
  Minerva::Core::Data::Style::RefPtr style ( _layer->style() );
  Minerva::Core::Data::LineStyle::RefPtr lineStyle;
  if ( style.valid() )
  {
    lineStyle = style->linestyle();
  }
  
  if ( !lineStyle )
  {
    lineStyle = new Minerva::Core::Data::LineStyle;
    lineStyle->mode ( Minerva::Core::Data::LineStyle::RANDOM );
  }
  
  // Get all geometries.
  while ( 0x0 != ( feature = layer->GetNextFeature() ) )
  {
    // Burn color.
    const Usul::Math::Vec4f color ( lineStyle->color() );

    // Add the burn values.
    _burnValues.push_back ( color[0] * 255 );
    _burnValues.push_back ( color[1] * 255 );
    _burnValues.push_back ( color[2] * 255 );
    _burnValues.push_back ( color[3] * 255 );

    // Add the geometry.
    OGRGeometry *geometry ( feature->GetGeometryRef() );

    if ( 0x0 != geometry )
      _geometries.push_back ( geometry->clone() );

    // Cleanup.
    OGRFeature::DestroyFeature( feature );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clone.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature* RasterPolygonLayer::clone() const
{
  return new RasterPolygonLayer( *this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the texture.
//
///////////////////////////////////////////////////////////////////////////////

RasterPolygonLayer::ImagePtr RasterPolygonLayer::_textureImplementation ( 
                                                                         const std::string& filename, 
                                                                         const TileKey& key,
                                                                         unsigned int width, 
                                                                         unsigned int height,  
                                                                         Usul::Jobs::Job *, 
                                                                         IUnknown *caller )
{
  // Make sure we are initialized.
  this->_init();

  // Initialize.
  ImagePtr image ( this->_rasterize ( key.extents(), width, height, key.level() ) );
    
  // Cache the file.
  this->_writeImageFile ( image, filename );
  
  return image;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Scoped close.
//
///////////////////////////////////////////////////////////////////////////////

namespace Minerva {
namespace Detail
{
  struct ScopedDataset
  {
    ScopedDataset ( GDALDataset * data ) : _data ( data )
    {
    }
    ~ScopedDataset ()
    {
      if ( 0x0 != _data )
        ::GDALClose ( _data );
    }
    
  private:
    GDALDataset *_data;
  };
}
}


///////////////////////////////////////////////////////////////////////////////
//
//  Rasterize.
//
///////////////////////////////////////////////////////////////////////////////

RasterPolygonLayer::ImagePtr RasterPolygonLayer::_rasterize ( const Extents& extents, unsigned int width, unsigned int height, unsigned int level )
{
  // Set an error handler.
  Detail::PushPopErrorHandler handler;

  // How many channels do we want.
  const unsigned int channels ( 4 );

  std::vector<char> bytes ( width * height * channels, 0 );
    
  // Typedefs.
  typedef std::vector<int> Bands;
  
  ImagePtr image ( 0x0 );

  Guard guard ( this );
  
  // Make an in memory raster.
  std::string format ( "MEM" );
  
  // Find a drive for geotiff.
  GDALDriver *driver ( GetGDALDriverManager()->GetDriverByName ( format.c_str() ) );
  
  // Return now if we didn't find a driver.
  if ( 0x0 == driver )
    return 0x0;
  
  // Make a temp file.
  Usul::File::Temp temp;
  
  // Create the file.
  GDALDataset *data ( driver->Create( temp.name().c_str(), width, height, channels, GDT_Byte, 0x0 ) );
  
  if ( 0x0 == data )
    return 0x0;

  // Make sure the dataset is closed.
  Detail::ScopedDataset scoped ( data );

  // Get the extents lower left and upper right.
  Extents::Vertex ll ( extents.minimum() );
  Extents::Vertex ur ( extents.maximum() );
  
  // Make the transform.
  OGRSpatialReference dst ( _projectionText.c_str() ), src;
  src.SetWellKnownGeogCS ( "WGS84" );
  OGRCoordinateTransformation* transform ( ::OGRCreateCoordinateTransformation ( &src, &dst ) );
  
  // Make sure the transformation is destroyed.
  Usul::Scope::Caller::RefPtr destroyTransform ( Usul::Scope::makeCaller ( boost::bind<void> ( ::OCTDestroyCoordinateTransformation, transform ) ) );
  
  // Set the extents.
  if ( 0x0 != transform )
  {
    transform->Transform ( 1, &ll[0], &ll[1] );
    transform->Transform ( 1, &ur[0], &ur[1] );
  }
  
  // Get the length in x and y.
  const double xLength ( ur[0] - ll[0] );
  const double yLength ( ur[1] - ll[1] );
  
  // Figure out the degrees per pixel.
  const double xResolution  ( xLength / width );
  const double yResolution  ( yLength / height );
  
  std::vector<double> geoTransform ( 6 );
  
  geoTransform[0] = ll[0];          // top left x
  geoTransform[1] = xResolution;    // w-e pixel resolution
  geoTransform[2] = 0;              // rotation, 0 if image is "north up"
  geoTransform[3] = ur[1];          // top left y
  geoTransform[4] = 0;              // rotation, 0 if image is "north up"
  geoTransform[5] = -yResolution;   // n-s pixel resolution
  
  if ( CE_None != data->SetGeoTransform( &geoTransform[0] ) )
    return 0x0;
  
  if ( CE_None != data->SetProjection( Usul::Threads::Safe::get ( this->mutex(), _projectionText ).c_str() ) )
    return 0x0;
  
  // Bands to burn.
  Bands bands;
  
  // Fill in the values.
  for ( unsigned int i = 0; i < channels; ++i )
  {
    // GDAL band index starts at 1.
    bands.push_back ( i + 1 );
  }

  if ( CE_None != data->RasterIO( GF_Write, 0, 0, width, height, &bytes[0], width, height, GDT_Byte, channels, 0x0, 0, 0, 0 ) )
    return 0x0;

  // Burn the raster.
  if ( CE_None == ::GDALRasterizeGeometries ( 
    data, bands.size(), &bands[0], _geometries.size(), (void**) &_geometries[0], 
    0x0, 0x0, &_burnValues[0], 0x0, GDALTermProgress, 0x0 ) )
  {
    image = Minerva::GDAL::makeImage ( width, height, 4, GDT_Byte );
    Minerva::convert<unsigned char> ( image.get(), data, GDT_Byte );
  }

  return image;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize.
//
///////////////////////////////////////////////////////////////////////////////

void RasterPolygonLayer::deserialize ( const XmlTree::Node &node )
{
  Guard guard ( this );
  
  _dataMemberMap.deserialize ( node );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the directory.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Common::LayerKey::RefPtr RasterPolygonLayer::cacheKey() const
{
  const std::string name ( "rasterized" );
  
  Guard guard ( this );
  const std::string layerName ( _layer.valid() ? _layer->name() : "" );
  std::size_t id ( BaseClass::_hashString ( layerName ) );

  return new LayerKey ( name, id );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extension for the cached files.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterPolygonLayer::_cacheFileExtension() const
{
  return "tif";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Write image file.
//
///////////////////////////////////////////////////////////////////////////////

void RasterPolygonLayer::_writeImageFile ( ImagePtr image, const std::string& filename ) const
{
  if ( image.valid() )
  {
    osgDB::writeImageFile ( *image, filename );
  }
}

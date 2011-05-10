
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Plugins/GDAL/RasterLayerGDAL.h"
#include "Minerva/Plugins/GDAL/Convert.h"
#include "Minerva/Plugins/GDAL/Common.h"
#include "Minerva/Plugins/GDAL/MakeImage.h"

#include "Minerva/Core/ElevationData.h"
#include "Minerva/Core/DiskCache.h"

#include "Usul/Factory/RegisterCreator.h"
#include "Usul/File/Path.h"
#include "Usul/File/Temp.h"
#include "Usul/Math/Interpolate.h"
#include "Usul/MPL/StaticAssert.h"
#include "Usul/Predicates/CloseFloat.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Scope/RemoveFile.h"
#include "Usul/Strings/Format.h"
#include "Usul/Threads/Safe.h"

#include "boost/bind.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/math/special_functions/round.hpp"

#include "gdal.h"
#include "gdal_priv.h"
#include "gdal_alg.h"
#include "gdalwarper.h"
#include "ogr_api.h"
#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "cpl_error.h"

#include <cmath>

using namespace Minerva::Layers::GDAL;

USUL_FACTORY_REGISTER_CREATOR_WITH_NAME ( "RasterLayerGDAL", RasterLayerGDAL );


///////////////////////////////////////////////////////////////////////////////
//
//  Default Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerGDAL::RasterLayerGDAL() : 
  BaseClass(),
  _data ( 0x0 ),
  _warpedData ( 0x0 ),
  _geoTransform ( 6, 0 ),
  _invGeoTransform ( 6, 0 ),
  _filename()
{
  this->_addMember ( "filename", _filename );
  
  // Sanity check.  TODO: Change from using built in types directy to Usul::Types
  USUL_STATIC_ASSERT ( sizeof ( GByte )   == sizeof ( unsigned char ) );
  USUL_STATIC_ASSERT ( sizeof ( GUInt16 ) == sizeof ( unsigned short ) );
  USUL_STATIC_ASSERT ( sizeof ( GInt16 )  == sizeof ( short ) );
  USUL_STATIC_ASSERT ( sizeof ( GInt32 )  == sizeof ( int ) );
  USUL_STATIC_ASSERT ( sizeof ( GUInt32 ) == sizeof ( unsigned int ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerGDAL::RasterLayerGDAL ( const RasterLayerGDAL& rhs ) :
  BaseClass ( rhs ),
  _data ( rhs._data ),
  _warpedData ( rhs._warpedData ),
  _filename ( rhs._filename )
{
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerGDAL::~RasterLayerGDAL()
{
  const bool same ( _warpedData == _data );
  if ( 0x0 != _warpedData )
    ::GDALClose ( _warpedData );

  // Clean up.
  if ( 0x0 != _data && !same )
    ::GDALClose ( _data );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clone.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Core::Data::Feature* RasterLayerGDAL::clone() const
{
  return new RasterLayerGDAL( *this );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the texture.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerGDAL::ImagePtr RasterLayerGDAL::_textureImplementation ( 
                                                                   const std::string& file, 
                                                                   const TileKey& key, 
                                                                   unsigned int width, 
                                                                   unsigned int height, 
                                                                   Usul::Jobs::Job * job,
                                                                   IUnknown *caller )
{
  SCOPED_GDAL_LOCK;
  
  // Create the dataset.
  Dataset::RefPtr tile ( this->_createTile ( "", key.extents(), width, height ) );
  if ( false == tile.valid() )
    return 0x0;

  // Convert to an osg image.
  ImagePtr image ( Minerva::convert ( tile->dataset() ) );

  // Cache the image.
  const std::string format ( "GTiff" );

  // Find a driver for in memory raster.
  GDALDriver *driver ( GetGDALDriverManager()->GetDriverByName ( format.c_str() ) );

  if ( driver )
  {
    GDALDataset *cacheFile ( driver->CreateCopy ( file.c_str(), tile->dataset(), FALSE, NULL, NULL, NULL ) );

    // Close the dataset to write to file.
    if ( cacheFile != 0x0 )
      ::GDALClose( (GDALDatasetH) cacheFile );
  }
  //Minerva::Core::DiskCache::instance().writeImage ( file, image );

  // Close the file.
  tile->close();

  return image;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the raster data as elevation data.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayerGDAL::IElevationData::RefPtr RasterLayerGDAL::elevationData ( 
  const TileKey& key,
  unsigned int width,
  unsigned int height,
  Usul::Jobs::Job* job,
  Usul::Interfaces::IUnknown* caller )
{
  SCOPED_GDAL_LOCK;
  
  Extents extents ( key.extents() );
#if 0
  // Get the filename for the cache.
  std::string file;
  CacheStatus status ( this->_getAndCheckCacheFilename ( extents, width, height, level, file ) );
  if ( CACHE_STATUS_FILE_OK == status )
  {
    // Open the dataset.
    Dataset::RefPtr data ( new Dataset ( static_cast< GDALDataset* > ( ::GDALOpen ( file.c_str(), GA_ReadOnly ) ) ) );
    return data->convertToElevationData();
  }

  const std::string tempFilename ( Usul::File::Temp::file() );
  Usul::Scope::RemoveFile removeFile ( tempFilename );
#endif
  
  // Now guard.
  Guard guard ( this );

  // Get the data set.
  GDALDataset *data ( _warpedData );

  // Return if no data.
  if ( 0x0 == data )
    return 0x0;
  
  // Get the number of bands.
  const int bands ( data->GetRasterCount() );
  
  // Return if we don't have any bands.
  if ( 0 == bands )
    return 0x0;

  GDALRasterBand* band ( data->GetRasterBand ( 1 ) );

  int hasNoData ( FALSE );
  double noDataValue ( band->GetNoDataValue( &hasNoData ) );
  const double defaultNoDataValue ( -6666.0 );

  if ( FALSE == hasNoData )
  {
    noDataValue = defaultNoDataValue;
  }

  Minerva::Core::ElevationData::RefPtr elevationData ( new Minerva::Core::ElevationData ( width, height ) );
  elevationData->noDataValue ( noDataValue );

  const Extents::Vertex &mn ( extents.minimum() );
  const Extents::Vertex &mx ( extents.maximum() );

  for ( unsigned int i = 0; i < width; ++i )
  {
    const double u ( static_cast<double> ( i ) / ( width - 1 ) );

    for ( unsigned int j = 0; j < height; ++j )
    {
      const double v ( static_cast<double> ( j ) / ( height - 1 ) );
      
      const double lon ( mn[0] + u * ( mx[0] - mn[0] ) );
      const double lat ( mn[1] + v * ( mx[1] - mn[1] ) );

      const float result ( this->_getElevationData ( band, lon, lat, noDataValue ) );

      elevationData->value ( i, j, result );
    }
  }

  //boost::filesystem::copy_file ( tempFilename, file );

  return Minerva::Common::IElevationData::RefPtr ( elevationData );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Clamp the index to the edge, accounting for the half pixel difference.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail {
  void clampIndex ( double &index, int size )
  {
    if ( index < 0 && index >= -0.5 )
    {
        index = 0;
    }
    else if ( index > size - 1 && index <= size - 0.5 )
    {
        index = size - 1;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Move the row and column to the center of the pixel.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail {
  void moveToCenterOfPixel ( double& row, double& column, int width, int height )
  {
    // Move the center of the pixel to the upper left corner (where the vertex of the mesh is located).
    row -= 0.5;
    column -= 0.5;

    Detail::clampIndex ( column, width );
    Detail::clampIndex ( row, height );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the elevation value at given longitude,latitude.
//
///////////////////////////////////////////////////////////////////////////////

float RasterLayerGDAL::_getElevationData ( GDALRasterBand* band, double longitude, double latitude, float noDataValue )
{
  Guard guard ( this );

  double currentRow ( 0 ), currentColumn ( 0 );
  ::GDALApplyGeoTransform ( &_invGeoTransform[0], longitude, latitude, &currentColumn, &currentRow );

  const int width ( band->GetXSize() );
  const int height ( band->GetYSize() );

  // Since we are getting elevation data, move to the center of the pixel.  (The vertex in triangle mesh is at the center of pixel.)
  ::Detail::moveToCenterOfPixel ( currentRow, currentColumn, width, height );

  // If our index is out side the bounds, return the no data value.
  if ( currentRow < 0 || currentColumn < 0 || currentRow > ( height - 1 ) || currentColumn > ( width - 1 ) )
    return noDataValue;

  // Get the lower left and upper right indices around point.
  const int llRow    ( Usul::Math::maximum ( static_cast<int> ( ::floor ( currentRow ) ), 0 ) );
  const int llColumn ( Usul::Math::maximum ( static_cast<int> ( ::floor ( currentColumn ) ), 0) );

  const int urRow    ( Usul::Math::maximum ( Usul::Math::minimum ( static_cast<int> ( ::ceil ( currentRow ) ), ( height - 1 ) ), 0 ) );
  const int urColumn ( Usul::Math::maximum ( Usul::Math::minimum ( static_cast<int> ( ::ceil ( currentColumn ) ), ( width - 1 ) ), 0) );

  // Get the four values needed.
  double urHeight ( noDataValue ), llHeight ( noDataValue ), ulHeight ( noDataValue ), lrHeight ( noDataValue );
  band->RasterIO ( GF_Read, llColumn, llRow, 1, 1, &llHeight, 1, 1, GDT_Float64, 0, 0 );
  band->RasterIO ( GF_Read, llColumn, urRow, 1, 1, &ulHeight, 1, 1, GDT_Float64, 0, 0 );
  band->RasterIO ( GF_Read, urColumn, llRow, 1, 1, &lrHeight, 1, 1, GDT_Float64, 0, 0 );
  band->RasterIO ( GF_Read, urColumn, urRow, 1, 1, &urHeight, 1, 1, GDT_Float64, 0, 0 );

  // Don't interpolate if any values are no data.
  Usul::Predicates::CloseFloat<double> close ( 3 );
  if ( close ( llHeight, noDataValue ) || close ( ulHeight, noDataValue ) || close ( lrHeight, noDataValue ) || close ( urHeight, noDataValue ) )
    return noDataValue;

  // Bilinear interpolate ( calculated using a series of linear interpolations. ).
  const double u ( currentColumn - static_cast<float> ( llColumn ) );
  const double v ( currentRow - static_cast<float> ( llRow ) );

  const double result0 ( Usul::Math::Interpolate<double>::linear ( u, llHeight, lrHeight ) );
  const double result1 ( Usul::Math::Interpolate<double>::linear ( u, ulHeight, urHeight ) );

  const double result ( Usul::Math::Interpolate<double>::linear ( v, result0, result1 ) );

  return static_cast<float> ( result );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create raster data for given extents.
//
///////////////////////////////////////////////////////////////////////////////

Dataset::RefPtr RasterLayerGDAL::_createTile ( 
    const std::string& filename, 
    const Extents& extents, 
    unsigned int requestedWidth, 
    unsigned int requestedHeight )
{
  int target_width = requestedWidth;
  int target_height = requestedHeight;
  int tile_offset_left = 0;
  int tile_offset_top = 0;

  std::vector<double> geoTransform ( 6 );

  {
    Guard guard ( this );
    geoTransform.assign ( _geoTransform.begin(), _geoTransform.end() );
  }

  // Modified from gdal_translate.cpp
  int anSrcWin[4];
  anSrcWin[0] = (int) ((extents[0] - geoTransform[0]) / geoTransform[1] + 0.001);
  anSrcWin[1] = (int) ((extents[3] - geoTransform[3]) / geoTransform[5] + 0.001);

  anSrcWin[2] = (int) ((extents[2] - extents[0]) / geoTransform[1] + 0.5);
  anSrcWin[3] = (int) ((extents[1] - extents[3]) / geoTransform[5] + 0.5);

#if 0
  int off_x = int((extents[0] - _geoTransform[0]) / _geoTransform[1]);
  int off_y = int((extents[3] - _geoTransform[3]) / _geoTransform[5]);
  int width = int(((extents[2] - _geoTransform[0]) / _geoTransform[1]) - off_x);
  int height = int(((extents[1] - _geoTransform[3]) / _geoTransform[5]) - off_y);
#else
  int off_x = anSrcWin[0];
  int off_y = anSrcWin[1];
  int width = anSrcWin[2];
  int height = anSrcWin[3];
#endif

  const int rasterXSize ( _warpedData->GetRasterXSize() );
  const int rasterYSize ( _warpedData->GetRasterYSize() );

  if (off_x + width > rasterXSize )
  {
    const int oversize_right ( off_x + width - rasterXSize );
    target_width = target_width - int(float(oversize_right) / width * target_width);
    width = rasterXSize - off_x;
  }

  if (off_x < 0)
  {
    int oversize_left = -off_x;
    tile_offset_left = int ( float(oversize_left) / width * target_width);
    target_width = target_width - int(float(oversize_left) / width * target_width);
    width = width + off_x;
    off_x = 0;
  }

  if (off_y + height > rasterYSize )
  {
    const int oversize_bottom ( off_y + height - rasterYSize );
    target_height = target_height - boost::math::round(float(oversize_bottom) / height * target_height);
    height = rasterYSize - off_y;
  }

  if (off_y < 0)
  {
    int oversize_top = -off_y;
    tile_offset_top = int(float(oversize_top) / height * target_height);
    target_height = target_height - int(float(oversize_top) / height * target_height);
    height = height + off_y;
    off_y = 0;
  }

  // Get the data set.
  GDALDataset *data ( Usul::Threads::Safe::get ( this->mutex(), _warpedData ) );

  // Return if no data.
  if ( 0x0 == data )
    return 0x0;

  // This shouldn't happen, but does, so I think the logic above is incorrect.
    if ( width <=0 || height <=0 || target_width <=0 || target_height <=0 )
      return 0x0;

  // Get the number of bands.
  const int bands ( data->GetRasterCount() );

  const bool needAlpha ( 3 == bands || 1 == bands );
  const int numberOfBands ( needAlpha ? bands + 1 : bands );

  Dataset::RefPtr dataset ( new Dataset ( filename, extents, requestedWidth, requestedHeight, numberOfBands, GDT_Byte ) );
  GDALDataset *tile ( dataset->dataset() );

  std::vector<unsigned char> buffer ( target_width * target_height, 0 );

  for ( int i = 1; i <= bands; ++i )
  {
    GDALRasterBand* band ( data->GetRasterBand ( i ) );
    {
      Guard guard ( this );
      band->RasterIO ( GF_Read, off_x, off_y, width, height, &buffer[0], target_width, target_height, GDT_Byte, 0, 0 );
    }
    tile->GetRasterBand ( i )->RasterIO ( GF_Write, tile_offset_left, tile_offset_top, target_width, target_height, &buffer[0], target_width, target_height, GDT_Byte, 0, 0 );
  }
  
  if ( needAlpha )
  {
    std::vector<unsigned char> alphas ( target_width * target_height, 255 );
    tile->GetRasterBand ( bands + 1 )->RasterIO ( GF_Write, tile_offset_left, tile_offset_top, target_width, target_height, &alphas[0], target_width, target_height, GDT_Byte, 0, 0 );
  }

  return dataset;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Deserialize.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerGDAL::deserialize ( const XmlTree::Node &node )
{
  Guard guard ( this );
  
  _dataMemberMap.deserialize ( node );
  
  // Read.
  this->read ( Usul::Threads::Safe::get ( this->mutex(), _filename ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayerGDAL::read ( const std::string& filename )
{
  SCOPED_GDAL_LOCK;

  // Add an error handler.
  Minerva::Detail::PushPopErrorHandler error;
  
  if ( true == this->name().empty() )
    this->name ( filename );
  
  Guard guard ( this );
  _filename = filename;
  
  // Open the dataset.
  _data = static_cast< GDALDataset* > ( ::GDALOpen ( filename.c_str(), GA_ReadOnly ) );
  
  if ( 0x0 != _data )
  {
    const char* projection ( _data->GetProjectionRef() );
    OGRSpatialReference src ( projection );

    if ( false == src.IsGeographic() )
    {
      OGRSpatialReference dst;
      dst.SetWellKnownGeogCS ( "WGS84" );

      char *dstProjection ( 0x0 );
      dst.exportToWkt ( &dstProjection );

      _warpedData = (GDALDataset*) GDALAutoCreateWarpedVRT (
                      _data,
                      projection,
                      dstProjection,
                      /*GRA_NearestNeighbour*/ GRA_Bilinear,
                      5.0,
                      NULL);

      ::CPLFree ( dstProjection );
    }
    else
    {
      _warpedData = _data;
    }

    if ( CE_None == _warpedData->GetGeoTransform ( &_geoTransform[0] ) )
    {
      const unsigned int width ( _warpedData->GetRasterXSize() );
      const unsigned int height ( _warpedData->GetRasterYSize() );
      
      const double x ( _geoTransform[0] ), y ( _geoTransform[3] );
      
      const double xLength ( width * _geoTransform[1] );
      const double yLength ( height * _geoTransform[5] );
      
      Extents::Vertex ll ( x, y + yLength ), ur ( x + xLength, y );
      
      Extents extents ( ll, ur );
      this->extents ( extents );
    }

    // Store the inverse of the geo tranform.
    ::GDALInvGeoTransform ( &_geoTransform[0], &_invGeoTransform[0] );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the directory.
//
///////////////////////////////////////////////////////////////////////////////

Minerva::Common::LayerKey::RefPtr RasterLayerGDAL::cacheKey() const
{
  const std::string name ("file_system" );
  
  const std::string file ( Usul::Threads::Safe::get ( this->mutex(), _filename ) );
  const std::size_t id ( BaseClass::_hashString ( Usul::File::fullPath ( file ) ) );

  return new LayerKey ( name, id );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the extension for the cached files.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerGDAL::_cacheFileExtension() const
{
  // Always save as tiff.
  return "tif";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the size of underlying image.
//
///////////////////////////////////////////////////////////////////////////////

Usul::Math::Vec2ui RasterLayerGDAL::size() const
{
  Guard guard ( this );
  Usul::Math::Vec2ui size ( 0, 0 );

  if ( 0x0 != _data )
  {
    size[0] = _data->GetRasterXSize();
    size[1] = _data->GetRasterYSize();
  }

  return size;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the projection.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayerGDAL::projection() const
{
  Guard guard ( this );
  if ( 0x0 != _data )
  {
    return std::string ( _data->GetProjectionRef() );
  }

  return "";
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Arizona State University
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//  Author: Adam Kubach
//
///////////////////////////////////////////////////////////////////////////////

#include "Minerva/Core/Layers/RasterLayer.h"
#include "Minerva/Core/DiskCache.h"
#include "Minerva/Core/ElevationData.h"
#include "Minerva/Core/Visitor.h"

#include "Usul/Components/Manager.h"
#include "Usul/Functions/Color.h"
#include "Usul/Functions/SafeCall.h"
#include "Usul/Jobs/Job.h"
#include "Usul/Scope/Caller.h"
#include "Usul/Strings/Format.h"
#include "Usul/Threads/ThreadId.h"

#include "osg/Image"

#include "boost/functional/hash.hpp"

#include <algorithm>
#include <limits>
#include <ctime>
#include <cstring>

using namespace Minerva::Core::Layers;


///////////////////////////////////////////////////////////////////////////////
//
//  Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::RasterLayer() : 
  BaseClass(),
  _alphas(),
  _alpha ( 1.0f ),
  _reader ( 0x0 ),
  _log ( 0x0 ),
  _levelRange ( 0, std::numeric_limits<unsigned int>::max() )
{
  this->_registerMembers();

  // Initialize the reader.
  this->_imageReaderFind ( "png" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Copy Constructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::RasterLayer ( const RasterLayer& rhs ) : BaseClass ( rhs ),
  _alphas ( rhs._alphas ),
  _alpha ( rhs._alpha ),
  _reader ( rhs._reader ),
  _log ( rhs._log ),
  _levelRange ( rhs._levelRange )
{
  this->_registerMembers();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Destructor.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::~RasterLayer()
{
  _alphas.clear();
  _reader = 0x0;
  _log = 0x0;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Register members for serialization.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::_registerMembers()
{
  // Serialization glue.
  this->_addMember ( new Serialize::XML::ValueMapMember<Alphas> ( "alphas", _alphas ) );
  this->_addMember ( "alpha", _alpha );
  this->_addMember ( "level_range", _levelRange );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get overall alpha value.
//
///////////////////////////////////////////////////////////////////////////////

float RasterLayer::alpha() const
{
  Guard guard ( this );
  return _alpha;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set overall alpha value.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::alpha ( float a )
{
  Guard guard ( this );
  _alpha = a;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Add an alpha value.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::alpha ( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha )
{
  Guard guard ( this );
  Color color ( Usul::Functions::Color::pack ( red, green, blue, 0 ) );
  _alphas[color] = alpha;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the alpha values.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::Alphas RasterLayer::alphas() const
{
  Guard guard ( this );
  return _alphas;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the alpha values.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::alphas( const Alphas& alphas )
{
  Guard guard ( this );
  _alphas = alphas;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the texture.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::ImagePtr RasterLayer::texture ( 
                                            const TileKey& key,  
                                            unsigned int width,
                                            unsigned int height,
                                            Usul::Jobs::Job *job, 
                                            IUnknown * )
{
  Extents extents ( key.extents() );
  
  // Check to make sure we have sane extents.
  if ( extents.minimum()[0] < -180.0 || extents.minimum()[1] < -90.0 || extents.maximum()[0] > 180.0 || extents.maximum()[1] > 90.0 )
    return ImagePtr ( 0x0 );

  // See if the job has been cancelled.
  RasterLayer::_checkForCanceledJob ( job );

  // Make the file name.
  std::string file;
  if ( DiskCache::CACHE_STATUS_FILE_OK == this->_getAndCheckCacheFilename ( key, width, height, file ) )
  {
    RasterLayer::_checkForCanceledJob ( job );

    // Load the file.
    ImagePtr image ( this->_readImageFile ( file ) );
    return image;
  }

  return this->_textureImplementation ( file, key, width, height, job, 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the cache filename.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::DiskCache::CacheStatus RasterLayer::_getAndCheckCacheFilename (
                                                                 const TileKey& key, 
                                                                 unsigned int width,
                                                                 unsigned int height, 
                                                                 std::string& filename )
{
  // Get the cache key.
  LayerKey::RefPtr layerKey ( this->cacheKey() );
  return Minerva::Core::DiskCache::instance().getAndCheckCacheFilename ( *layerKey, key, width, height, this->_cacheFileExtension(), filename );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Default is to cache in PNG format.
//
///////////////////////////////////////////////////////////////////////////////

std::string RasterLayer::_cacheFileExtension() const
{
  return std::string ( "png" );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Check if the job has been marked as canceled. If so, cancel it, which 
//  should throw an exception.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::_checkForCanceledJob ( Usul::Jobs::Job *job )
{
  if ( ( 0x0 != job ) && ( true == job->canceled() ) )
    job->cancel();
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read an image file.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::ImagePtr RasterLayer::_readImageFile ( const std::string &file ) const
{
  return RasterLayer::_readImageFile ( file, this->_imageReaderGet() );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Read an image file.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::ImagePtr RasterLayer::_readImageFile ( const std::string &file, ReaderPtr reader )
{
  return Minerva::Core::DiskCache::instance().readImage ( file, reader );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the image reader.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::ReaderPtr RasterLayer::_imageReaderGet() const
{
  Guard guard ( this );
  return ReaderPtr ( _reader );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the image reader.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::_imageReaderSet ( ReaderPtr reader )
{
  Guard guard ( this );
  _reader = reader;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Find a reader for our format.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::_imageReaderFind ( const std::string &ext )
{
  // Typedefs to shorten the lines.
  typedef Usul::Components::Manager PluginManager;
  typedef PluginManager::UnknownSet Unknowns;

  // Get the list of unknowns that can read this format.
  Unknowns unknowns ( PluginManager::instance().getInterfaces ( IReadImageFile::IID ) );

  // Find the one that also handles the extension.
  for ( Unknowns::const_iterator iter = unknowns.begin(); iter != unknowns.end(); ++iter )
  {
    IReadImageFile::QueryPtr reader ( iter->get() );
    if ( reader.valid() && reader->canRead ( Usul::Strings::format ( ".", ext ) ) )
    {
      // Set the new image reader.
      this->_imageReaderSet ( reader.get() );
      break;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Return the hashed string.
//
///////////////////////////////////////////////////////////////////////////////

std::size_t RasterLayer::_hashString ( const std::string &s )
{
  typedef boost::hash<std::string> StringHash;
  StringHash hashFunction;
  const std::size_t value ( hashFunction ( s ) );

  return value;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Set the log.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::logSet ( LogPtr lp )
{
  Guard guard ( this );
  _log = lp;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the log.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::LogPtr RasterLayer::logGet()
{
  Guard guard ( this );
  return LogPtr ( _log );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Log the event.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::_logEvent ( const std::string &s )
{
  // Make the message.
  const std::string message ( Usul::Strings::format 
    ( "clock: ", ::clock(), ", system thread: ", Usul::Threads::currentThreadId(), ", event: ", s ) );

  // Get the log.
  LogPtr file ( this->logGet() );

  // Use the log if it's valid.
  if ( ( false == s.empty() ) && ( true == file.valid() ) )
  {
    file->write ( message );
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  See if the given level falls within this layer's range of levels.
//
///////////////////////////////////////////////////////////////////////////////

bool RasterLayer::isInLevelRange ( unsigned int level ) const
{
  Guard guard ( this );
  return ( ( level >= _levelRange[0] ) && ( level <= _levelRange[1] ) );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Create ElevationData from an osg::Image.  This is to support the legacy api and will be removed.
//
///////////////////////////////////////////////////////////////////////////////

namespace Detail {

  template < class SrcType >
  RasterLayer::IElevationData::RefPtr convertFromOsgImage ( const osg::Image& image )
  {
    const SrcType *src ( reinterpret_cast < const SrcType* > ( image.data()  ) );

    // Make sure the pointers are valid.
    if ( 0x0 == src )
      return 0x0;

    const unsigned int width ( image.s() );
    const unsigned int height ( image.t() );
    Minerva::Common::IElevationData::RefPtr data ( new Minerva::Core::ElevationData ( width, height ) );
    for ( unsigned int i = 0; i < width; ++i )
    {
      for ( unsigned int j = 0; j < height; ++j )
      {
        const SrcType value ( *reinterpret_cast < const SrcType * > ( image.data ( i, j ) ) );
        data->value ( i, j, static_cast<float> ( value ) );
      }
    }
    return data;
  }

  
  RasterLayer::IElevationData::RefPtr convertFromOsgImage ( osg::Image *image )
  {
    if ( 0x0 != image )
    {
      switch ( image->getDataType() )
      {
      // Treat unsigned shorts as shorts.  Any number greater than max short will be treated as a negative number.
      // This is a work around for one earth's wms server.
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
        return Detail::convertFromOsgImage<Usul::Types::Int16> ( *image );
      case GL_UNSIGNED_BYTE:
        return Detail::convertFromOsgImage<Usul::Types::Uint8> ( *image );
      case GL_FLOAT:
        return Detail::convertFromOsgImage<Usul::Types::Float32> ( *image );
      }
    }

    return 0x0;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
//  Get the raster data as elevation data.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::IElevationData::RefPtr RasterLayer::elevationData ( 
                                                                const TileKey& key,
                                                                unsigned int width,
                                                                unsigned int height,
                                                                Usul::Jobs::Job* job,
                                                                Usul::Interfaces::IUnknown* caller )
{
  osg::ref_ptr<osg::Image> image ( this->texture ( key, width, height, job, caller ) );
  Minerva::Common::IElevationData::RefPtr data ( Detail::convertFromOsgImage ( image.get() ) );
  return data;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Implementation to retrieve texture.
//
///////////////////////////////////////////////////////////////////////////////

RasterLayer::ImagePtr RasterLayer::_textureImplementation ( 
                                                           const std::string& filename, 
                                                           const TileKey& key,
                                                           unsigned int width,
                                                           unsigned int height,
                                                           Usul::Jobs::Job *, 
                                                           IUnknown *caller )
{
  return ImagePtr ( 0x0 );
}


///////////////////////////////////////////////////////////////////////////////
//
//  Accept the visitor.
//
///////////////////////////////////////////////////////////////////////////////

void RasterLayer::accept ( Minerva::Core::Visitor& visitor )
{
  visitor.visit ( *this );
}
